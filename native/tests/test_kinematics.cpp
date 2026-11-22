// SPDX-License-Identifier: GPL-3.0-or-later
#include "kinematics.hpp"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace {
using namespace physik;
int checks = 0;
std::map<std::string, Calculation> masks;

void need(bool condition, const std::string& message) {
  ++checks;
  if (!condition) {
    std::cerr << "FAIL KINEMATICS: " << message << '\n';
    std::exit(1);
  }
}

bool close(double actual, double expected, double relative = 2e-9,
           double absolute = 1e-10) {
  return std::isfinite(actual) && std::isfinite(expected) &&
         std::abs(actual - expected) <=
             std::max(absolute, relative * std::abs(expected));
}

void equal(double actual, double expected, const std::string& context,
           double relative = 2e-9, double absolute = 1e-10) {
  need(close(actual, expected, relative, absolute),
       context + ": expected " + formatNumber(expected) + ", got " +
           formatNumber(actual));
}

void checkResult(const KinematicResult& result,
                 const std::vector<KinematicValue>& expected,
                 const std::string& context, double relative = 2e-9,
                 double absolute = 1e-10) {
  need(result.values.size() == expected.size(), context + ": result count");
  std::set<std::string> names;
  for (size_t i = 0; i < expected.size(); ++i) {
    const auto& actual = result.values[i];
    need(names.insert(actual.name).second, context + ": unique result names");
    need(actual.name == expected[i].name, context + ": result name/order");
    need(actual.unit == expected[i].unit, context + ": unit of " + actual.name);
    equal(actual.value, expected[i].value, context + ": " + actual.name,
          relative, absolute);
  }
}

KinematicResult solve(const std::string& id, const std::vector<double>& input,
                      const std::string& context) {
  KinematicResult result;
  std::string error;
  const bool ok = calculateKinematics(masks.at(id), input, result, error);
  need(ok, context + " (" + id + "): " + error);
  need(error.empty(), context + ": no error on success");
  return result;
}

void expect(const std::string& id, const std::vector<double>& input,
            const std::vector<KinematicValue>& expected,
            const std::string& context, double relative = 2e-9,
            double absolute = 1e-10) {
  checkResult(solve(id, input, context), expected, context, relative, absolute);
}

void reject(const Calculation& calc, const std::vector<double>& input,
            const std::string& context) {
  KinematicResult result;
  std::string error;
  need(!calculateKinematics(calc, input, result, error),
       context + ": must reject, not invent a solution");
  need(!error.empty(), context + ": explanatory error");
}

void reject(const std::string& id, const std::vector<double>& input,
            const std::string& context) {
  reject(masks.at(id), input, context + " (" + id + ")");
}

void catalog() {
  const std::map<std::string, std::vector<Parameter>> expected = {
      {"KIN_STATE", {{"x0", "m", "m"}, {"v0", "m/s", "m/s"},
                     {"a", "m/s^2", "m/s^2"}, {"t", "s", "s"}}},
      {"KIN_T_X", {{"x0", "m", "m"}, {"v0", "m/s", "m/s"},
                   {"a", "m/s^2", "m/s^2"}, {"x", "m", "m"}}},
      {"KIN_T_V", {{"v0", "m/s", "m/s"}, {"a", "m/s^2", "m/s^2"},
                   {"v", "m/s", "m/s"}}},
      {"KIN_A_V", {{"v0", "m/s", "m/s"}, {"v", "m/s", "m/s"},
                   {"t", "s", "s"}}},
      {"KIN_A_X", {{"x0", "m", "m"}, {"v0", "m/s", "m/s"},
                   {"t", "s", "s"}, {"x", "m", "m"}}},
      {"KIN_V0_X", {{"x0", "m", "m"}, {"a", "m/s^2", "m/s^2"},
                    {"t", "s", "s"}, {"x", "m", "m"}}},
      {"KIN_A_DX", {{"v0", "m/s", "m/s"}, {"v", "m/s", "m/s"},
                    {"Delta_x", "m", "m"}}}};
  const auto& entries = kinematicsCatalog();
  need(entries.size() == 7, "exactly seven catalog entries");
  for (const auto& calc : entries) {
    need(expected.count(calc.id) == 1, "exact catalog ID " + calc.id);
    need(masks.emplace(calc.id, calc).second, "unique catalog ID " + calc.id);
    need(isKinematic(calc.id), "recognize catalog ID " + calc.id);
    const auto& parameters = expected.at(calc.id);
    need(calc.parameters.size() <= 4, calc.id + ": at most four parameters");
    need(calc.parameters.size() == parameters.size(), calc.id + ": arity");
    for (size_t i = 0; i < parameters.size(); ++i) {
      const auto& actual = calc.parameters[i];
      const auto& symbol = parameters[i].name;
      need(actual.name == symbol || actual.name.ends_with(" " + symbol),
           calc.id + ": parameter symbol/order");
      need(actual.unit == parameters[i].unit, calc.id + ": parameter base unit");
      const auto options = kinematicsUnits(actual);
      need(std::any_of(options.begin(), options.end(), [&](const Unit& unit) {
        return unit.name == actual.defaultUnit;
      }), calc.id + ": default unit is selectable");
    }
  }
  for (const auto& entry : expected)
    need(masks.count(entry.first) == 1, "catalog includes " + entry.first);
  for (const std::string id : {"", "KIN_", "KIN_STATE_EXTRA", "kin_state",
                               "KIN_UNKNOWN", "T_FROM_F", "FIT_PLANCK"})
    need(!isKinematic(id), "reject non-catalog ID " + id);
}

double converted(const Parameter& parameter, const std::string& unitName,
                 const std::string& text) {
  const auto options = kinematicsUnits(parameter);
  const auto found = std::find_if(options.begin(), options.end(), [&](const Unit& unit) {
    return unit.name == unitName;
  });
  need(found != options.end(), "select unit " + unitName + " for " + parameter.name);
  double result = 0;
  std::string error;
  const bool ok = parseNumber(text, found->scale, result, error);
  need(ok, "parse " + text + " " + unitName + ": " + error);
  return result;
}

void unitChoices() {
  const std::map<std::string, std::map<std::string, double>> expected = {
      {"m", {{"m", 1}, {"km", 1000}, {"cm", .01}, {"mm", .001}}},
      {"s", {{"s", 1}, {"min", 60}, {"h", 3600}, {"ms", .001}}},
      {"m/s", {{"m/s", 1}, {"km/h", 1.0 / 3.6}}},
      {"m/s^2", {{"m/s^2", 1}}}};
  for (const auto& entry : expected) {
    const Parameter parameter{"test", entry.first, entry.first};
    const auto options = kinematicsUnits(parameter);
    need(options.size() == entry.second.size(), entry.first + ": exact unit choices");
    std::set<std::string> names;
    for (const auto& unit : options) {
      need(names.insert(unit.name).second, entry.first + ": unique unit option");
      need(entry.second.count(unit.name) == 1, entry.first + ": unit name");
      equal(unit.scale, entry.second.at(unit.name), unit.name + ": scale", 1e-13, 1e-15);
      equal(converted(parameter, unit.name, "-2,5"), -2.5 * entry.second.at(unit.name),
            unit.name + ": signed decimal-comma conversion", 1e-13, 1e-15);
    }
  }

  const auto& state = masks.at("KIN_STATE").parameters;
  const std::vector<double> mixed = {
      converted(state[0], "km", "1"), converted(state[1], "km/h", "36"),
      converted(state[2], "m/s^2", "0"), converted(state[3], "h", "0.5")};
  const auto si = solve("KIN_STATE", {1000, 10, 0, 1800}, "SI uniform motion");
  const auto alternate = solve("KIN_STATE", mixed, "km, km/h, h uniform motion");
  const std::vector<KinematicValue> expectedUniform = {
      {"x", 19000, "m"}, {"Delta_x", 18000, "m"},
      {"v", 10, "m/s"}, {"Weg", 18000, "m"}};
  checkResult(si, expectedUniform, "SI independent expected values");
  checkResult(alternate, expectedUniform, "mixed units equal independent SI values");
  const auto& ax = masks.at("KIN_A_X").parameters;
  expect("KIN_A_X", {converted(ax[0], "km", "1"),
                      converted(ax[1], "km/h", "36"),
                      converted(ax[2], "h", "0.5"),
                      converted(ax[3], "km", "19")},
         {{"a", 0, "m/s^2"}, {"v", 10, "m/s"}}, "inverse mixed units");
}

void originals() {
  // AB_Überholvorgang_GK.pdf, PDF 4, Nr. 14 (Bob).
  // Original data: x0=0, v0=0, a=1.8 m/s^2, t=5 s; dx=22.5 m, v=9 m/s.
  expect("KIN_STATE", {0, 0, 1.8, 5},
         {{"x", 22.5, "m"}, {"Delta_x", 22.5, "m"},
          {"v", 9, "m/s"}, {"Weg", 22.5, "m"}}, "Bob original forward");
  // Derived inverse cases use Bob's independently calculated final x or v.
  expect("KIN_T_X", {0, 0, 1.8, 22.5},
         {{"t", 5, "s"}, {"v", 9, "m/s"}}, "Bob derived time from position");
  expect("KIN_T_V", {0, 1.8, 9},
         {{"t", 5, "s"}, {"Delta_x", 22.5, "m"}}, "Bob derived time from velocity");
  expect("KIN_A_V", {0, 9, 5},
         {{"a", 1.8, "m/s^2"}, {"Delta_x", 22.5, "m"}}, "Bob derived a from velocity");
  expect("KIN_A_X", {0, 0, 5, 22.5},
         {{"a", 1.8, "m/s^2"}, {"v", 9, "m/s"}}, "Bob derived a from position");
  expect("KIN_V0_X", {0, 1.8, 5, 22.5},
         {{"v0", 0, "m/s"}, {"v", 9, "m/s"}}, "Bob derived initial velocity");
  expect("KIN_A_DX", {0, 9, 22.5},
         {{"a", 1.8, "m/s^2"}, {"t", 5, "s"}}, "Bob derived a and t from dx");

  // ÜA_geradlinige Bewegung.pdf, PDF 2, Aufgabe Transrapid I.
  // 200 km/h = 500/9 m/s; a=(500/9)/61.1, dx=(250/9)*61.1.
  const auto& av = masks.at("KIN_A_V").parameters;
  const double transrapidV = converted(av[1], "km/h", "200");
  equal(transrapidV, 500.0 / 9, "Transrapid original speed conversion");
  expect("KIN_A_V", {0, transrapidV, 61.1},
         {{"a", .9092562284051646, "m/s^2"}, {"Delta_x", 1697.2222222222, "m"}},
         "Transrapid I original");
  expect("KIN_STATE", {0, 0, .9092562284051646, 61.1},
         {{"x", 1697.2222222222, "m"}, {"Delta_x", 1697.2222222222, "m"},
          {"v", 500.0 / 9, "m/s"}, {"Weg", 1697.2222222222, "m"}},
         "Transrapid I independent forward cross-check");

  // ÜA_geradlinige Bewegung.pdf, PDF 2, Aufgabe Gelbphase.
  // 54 -> 72 km/h over 25 m: (20^2-15^2)/(2*25)=3.5, t=10/7.
  const auto& adx = masks.at("KIN_A_DX").parameters;
  const double yellowV0 = converted(adx[0], "km/h", "54");
  const double yellowV = converted(adx[1], "km/h", "72");
  expect("KIN_A_DX", {yellowV0, yellowV, 25},
         {{"a", 3.5, "m/s^2"}, {"t", 10.0 / 7, "s"}}, "Gelbphase original 25 m");
  // Continue the SAME acceleration to x=37 m, not coasting at 20 m/s.
  expect("KIN_T_X", {0, 15, 3.5, 37},
         {{"t", 2, "s"}, {"v", 22, "m/s"}}, "Gelbphase accelerating through 37 m");
  expect("KIN_STATE", {0, 15, 3.5, 2},
         {{"x", 37, "m"}, {"Delta_x", 37, "m"},
          {"v", 22, "m/s"}, {"Weg", 37, "m"}}, "Gelbphase continued forward");
  expect("KIN_T_X", {25, 20, 3.5, 37},
         {{"t", 4.0 / 7, "s"}, {"v", 22, "m/s"}},
         "Gelbphase remaining 12 m still accelerating");

  // ÜA_geradlinige Bewegung.pdf, PDF 2, Aufgabe Schnellzug I.
  // 120 km/h = 100/3 m/s; a=-(100/3)^2/8000=-5/36, t=240 s.
  const double trainV = converted(adx[0], "km/h", "120");
  expect("KIN_A_DX", {trainV, 0, 4000},
         {{"a", -.1388888888888889, "m/s^2"}, {"t", 240, "s"}}, "Schnellzug I original");
  expect("KIN_T_V", {trainV, -5.0 / 36, 0},
         {{"t", 240, "s"}, {"Delta_x", 4000, "m"}}, "Schnellzug I stopping time");
  expect("KIN_STATE", {0, trainV, -5.0 / 36, 240},
         {{"x", 4000, "m"}, {"Delta_x", 4000, "m"},
          {"v", 0, "m/s"}, {"Weg", 4000, "m"}}, "Schnellzug I forward stop");
}

void signedMotionAndRoots() {
  expect("KIN_STATE", {-40, 3, 2, 4},
         {{"x", -12, "m"}, {"Delta_x", 28, "m"},
          {"v", 11, "m/s"}, {"Weg", 28, "m"}}, "nonzero signed initial position and speed");
  expect("KIN_STATE", {100, -10, 2, 8},
         {{"x", 84, "m"}, {"Delta_x", -16, "m"},
          {"v", 6, "m/s"}, {"Weg", 34, "m"}}, "reverse path is not absolute displacement");
  expect("KIN_STATE", {-100, 10, -2, 8},
         {{"x", -84, "m"}, {"Delta_x", 16, "m"},
          {"v", -6, "m/s"}, {"Weg", 34, "m"}}, "mirrored reverse path");
  expect("KIN_STATE", {5, -3, -2, 4},
         {{"x", -23, "m"}, {"Delta_x", -28, "m"},
          {"v", -11, "m/s"}, {"Weg", 28, "m"}}, "negative speed and acceleration");
  expect("KIN_STATE", {100, -10, 2, 5},
         {{"x", 75, "m"}, {"Delta_x", -25, "m"},
          {"v", 0, "m/s"}, {"Weg", 25, "m"}}, "stop at reversal");
  expect("KIN_T_X", {0, 10, -2, 16},
         {{"t_1", 2, "s"}, {"v_1", 6, "m/s"},
          {"t_2", 8, "s"}, {"v_2", -6, "m/s"}}, "both future roots in time order");
  expect("KIN_T_X", {0, 10, -2, 25},
         {{"t", 5, "s"}, {"v", 0, "m/s"}}, "double root emitted only once");
  expect("KIN_T_X", {100, -10, 2, 84},
         {{"t_1", 2, "s"}, {"v_1", -6, "m/s"},
          {"t_2", 8, "s"}, {"v_2", 6, "m/s"}}, "negative initial velocity future roots");
  expect("KIN_T_X", {0, 10, -2, -24},
         {{"t", 12, "s"}, {"v", -14, "m/s"}}, "only future root after reversal");
  expect("KIN_T_X", {0, 10, -2, 0},
         {{"t_1", 0, "s"}, {"v_1", 10, "m/s"},
          {"t_2", 10, "s"}, {"v_2", -10, "m/s"}}, "initial position now and after reversal");
  expect("KIN_T_X", {0, 0, -2, 0},
         {{"t", 0, "s"}, {"v", 0, "m/s"}}, "double root at time zero");
  expect("KIN_T_X", {-4, -3, 0, -19},
         {{"t", 5, "s"}, {"v", -3, "m/s"}}, "uniform negative linear root");
  expect("KIN_T_X", {10, 2, 0, 10},
         {{"t", 0, "s"}, {"v", 2, "m/s"}}, "uniform root at time zero");
  expect("KIN_STATE", {-7, -3, -2, 0},
         {{"x", -7, "m"}, {"Delta_x", 0, "m"},
          {"v", -3, "m/s"}, {"Weg", 0, "m"}}, "forward zero time");
  expect("KIN_STATE", {7, 0, 0, 8},
         {{"x", 7, "m"}, {"Delta_x", 0, "m"},
          {"v", 0, "m/s"}, {"Weg", 0, "m"}}, "stationary forward state");
  expect("KIN_T_V", {-10, 2, 6},
         {{"t", 8, "s"}, {"Delta_x", -16, "m"}}, "velocity inverse across reversal");
  expect("KIN_T_V", {3, -2, 3},
         {{"t", 0, "s"}, {"Delta_x", 0, "m"}}, "unique velocity time zero");
  expect("KIN_A_V", {-10, 6, 8},
         {{"a", 2, "m/s^2"}, {"Delta_x", -16, "m"}}, "signed acceleration from velocities");
  expect("KIN_A_X", {100, -10, 8, 84},
         {{"a", 2, "m/s^2"}, {"v", 6, "m/s"}}, "signed acceleration from position");
  expect("KIN_V0_X", {100, 2, 8, 84},
         {{"v0", -10, "m/s"}, {"v", 6, "m/s"}}, "negative recovered initial velocity");
  expect("KIN_A_DX", {-10, 6, -16},
         {{"a", 2, "m/s^2"}, {"t", 8, "s"}}, "signed displacement with reversal");
  expect("KIN_A_DX", {10, -6, 16},
         {{"a", -2, "m/s^2"}, {"t", 8, "s"}}, "negative acceleration with reversal");
  expect("KIN_A_DX", {-3, -3, -12},
         {{"a", 0, "m/s^2"}, {"t", 4, "s"}}, "equal nonzero velocities uniform motion");
  expect("KIN_A_V", {0, 0, 4},
         {{"a", 0, "m/s^2"}, {"Delta_x", 0, "m"}}, "known positive time stationary a");
  expect("KIN_A_X", {7, 0, 4, 7},
         {{"a", 0, "m/s^2"}, {"v", 0, "m/s"}}, "known positive time stationary position");
  expect("KIN_V0_X", {7, 0, 4, 7},
         {{"v0", 0, "m/s"}, {"v", 0, "m/s"}}, "known positive time stationary v0");
  expect("KIN_A_X", {7, 4, 2, 7},
         {{"a", -4, "m/s^2"}, {"v", -4, "m/s"}}, "zero displacement is possible with reversal");
}

void invalidAndExtreme() {
  reject("KIN_STATE", {0, 1, 2, -1}, "negative forward time");
  for (double t : {0.0, -1.0}) {
    reject("KIN_A_V", {0, 1, t}, "inverse given time must be positive");
    reject("KIN_A_X", {0, 1, t, 2}, "inverse given time must be positive");
    reject("KIN_V0_X", {0, 1, t, 2}, "inverse given time must be positive");
  }
  reject("KIN_T_X", {0, 10, -2, 26}, "target beyond turning point");
  reject("KIN_T_X", {0, 10, 2, -1}, "real roots only in past");
  reject("KIN_T_X", {0, 0, 2, -1}, "negative discriminant");
  reject("KIN_T_X", {10, 2, 0, 8}, "negative linear root");
  reject("KIN_T_X", {7, 0, 0, 7}, "stationary coincident target nonunique");
  reject("KIN_T_X", {7, 0, 0, 8}, "stationary disjoint target impossible");
  reject("KIN_T_V", {3, 0, 3}, "uniform coincident velocity nonunique");
  reject("KIN_T_V", {3, 0, 4}, "uniform different velocity impossible");
  reject("KIN_T_V", {3, 2, 1}, "velocity reached only in past");
  reject("KIN_A_DX", {0, 0, 1}, "stationary velocities cannot span distance");
  reject("KIN_A_DX", {3, -3, 1}, "opposite equal velocities require zero dx");
  reject("KIN_A_DX", {3, -3, 0}, "return trajectory leaves a and t nonunique");
  reject("KIN_A_DX", {3, 3, 0}, "zero time does not identify acceleration");
  reject("KIN_A_DX", {0, 0, 0}, "stationary dx leaves time nonunique");
  reject("KIN_A_DX", {1, 2, 0}, "nonzero velocity change at zero dx impossible");
  reject("KIN_A_DX", {3, 5, -8}, "distance implies negative time");

  const std::map<std::string, std::vector<double>> valid = {
      {"KIN_STATE", {0, 0, 1.8, 5}}, {"KIN_T_X", {0, 0, 1.8, 22.5}},
      {"KIN_T_V", {0, 1.8, 9}}, {"KIN_A_V", {0, 9, 5}},
      {"KIN_A_X", {0, 0, 5, 22.5}}, {"KIN_V0_X", {0, 1.8, 5, 22.5}},
      {"KIN_A_DX", {0, 9, 22.5}}};
  const double infinity = std::numeric_limits<double>::infinity();
  const double nan = std::numeric_limits<double>::quiet_NaN();
  for (const auto& entry : valid) {
    const auto& calc = masks.at(entry.first);
    reject(calc, {}, calc.id + ": empty inputs");
    auto input = entry.second;
    input.pop_back();
    reject(calc, input, calc.id + ": too few inputs");
    input = entry.second;
    input.push_back(1);
    reject(calc, input, calc.id + ": too many inputs");
    Calculation bad = calc;
    bad.parameters.push_back({"extra", "1", "1"});
    reject(bad, input, calc.id + ": matching but corrupted extra metadata/input");
    reject(bad, entry.second, calc.id + ": extra metadata with canonical input count");
    bad = calc;
    bad.parameters.pop_back();
    input = entry.second;
    input.pop_back();
    reject(bad, input, calc.id + ": matching but corrupted missing metadata/input");
    reject(bad, entry.second, calc.id + ": missing metadata with canonical input count");
    bad = calc;
    bad.parameters.clear();
    reject(bad, {}, calc.id + ": empty metadata and inputs");
    reject(bad, entry.second, calc.id + ": empty metadata with canonical input count");
    bad = calc;
    bad.id = "KIN_UNKNOWN";
    reject(bad, entry.second, calc.id + ": unknown formula metadata");
    for (size_t i = 0; i < entry.second.size(); ++i) {
      for (double invalid : {infinity, -infinity, nan}) {
        input = entry.second;
        input[i] = invalid;
        reject(calc, input, calc.id + ": nonfinite input " + std::to_string(i));
      }
    }
  }

  const double maximum = std::numeric_limits<double>::max();
  const double minimum = std::numeric_limits<double>::min();
  // These require at least one unrepresentable OUTPUT, not merely a large intermediate.
  reject("KIN_STATE", {maximum, maximum, maximum, 2}, "overflowing state outputs");
  reject("KIN_STATE", {-maximum, -maximum, -maximum, 2}, "negative state overflow");
  reject("KIN_T_X", {0, minimum, 0, maximum}, "linear time output overflows");
  reject("KIN_T_V", {0, minimum, maximum}, "velocity time output overflows");
  reject("KIN_A_V", {0, maximum, minimum}, "acceleration output overflows");
  reject("KIN_A_X", {0, 0, minimum, maximum}, "position acceleration output overflows");
  reject("KIN_V0_X", {0, 0, minimum, maximum}, "initial velocity output overflows");
  reject("KIN_A_DX", {0, maximum, minimum}, "distance acceleration output overflows");

  expect("KIN_T_X", {0, 1e8, 1, 1},
         {{"t", 1e-8, "s"}, {"v", 1e8, "m/s"}},
         "stable small positive quadratic root", 1e-12, 1e-20);
  expect("KIN_T_X", {0, 1e8, -1, 1},
         {{"t_1", 1e-8, "s"}, {"v_1", 1e8, "m/s"},
          {"t_2", 2e8, "s"}, {"v_2", -1e8, "m/s"}},
         "stable small root retained beside large future root", 1e-12, 1e-20);
  expect("KIN_T_X", {0, 0, 1e-12, .5},
         {{"t", 1e6, "s"}, {"v", 1e-6, "m/s"}},
         "small nonzero acceleration is not stationary", 1e-12, 1e-20);
  expect("KIN_T_V", {1, 1e-12, 2},
         {{"t", 1e12, "s"}, {"Delta_x", 1.5e12, "m"}},
         "small nonzero acceleration velocity inverse");
  expect("KIN_A_DX", {0, 1, 1e-12},
         {{"a", 5e11, "m/s^2"}, {"t", 2e-12, "s"}},
         "small nonzero displacement is not degenerate", 1e-12, 1e-24);
  expect("KIN_A_V", {0, 1, 1e-12},
         {{"a", 1e12, "m/s^2"}, {"Delta_x", 5e-13, "m"}},
         "strictly positive tiny given time", 1e-12, 1e-25);
  // Finite final answers remain valid even when naive t*t or v*v overflows.
  expect("KIN_STATE", {0, 0, 1e-200, 1e200},
         {{"x", 5e199, "m"}, {"Delta_x", 5e199, "m"},
          {"v", 1, "m/s"}, {"Weg", 5e199, "m"}},
         "large time with finite state outputs");
  expect("KIN_T_X", {0, 0, 1e200, 1e200},
         {{"t", std::sqrt(2.0), "s"}, {"v", std::sqrt(2.0) * 1e200, "m/s"}},
         "overflowing naive discriminant with finite roots");
  expect("KIN_T_X", {0, 0, 1e-200, 1e-200},
         {{"t", std::sqrt(2.0), "s"}, {"v", std::sqrt(2.0) * 1e-200, "m/s"}},
         "underflowing naive discriminant with finite roots", 2e-9, 1e-214);

  KinematicResult reused;
  std::string error;
  need(calculateKinematics(masks.at("KIN_T_X"), {0, 10, -2, 16}, reused, error),
       "reuse result: initial two-root call");
  need(calculateKinematics(masks.at("KIN_T_X"), {0, 10, -2, 25}, reused, error),
       "reuse result: single-root call");
  checkResult(reused, {{"t", 5, "s"}, {"v", 0, "m/s"}},
              "result replaces previous two roots");
}

void reviewRegressions() {
  reject("KIN_T_X", {-9007199254740992., 134217728., -1, 1},
         "target 1 m is above the exact maximum at 0 m despite rounded displacement");
  KinematicResult result;
  std::string error;
  const bool ok = calculateKinematics(masks.at("KIN_STATE"),
      {-9007199254740992., 9007199254740992., 2, 1}, result, error);
  if (ok) {
    need(result.values[0].name == "x", "cancellation case x result");
    equal(result.values[0].value, 1, "retain position residual or reject insufficient precision", 1e-14, 0);
  } else {
    need(!error.empty(), "explicit precision error instead of false zero position");
  }
  expect("KIN_STATE", {-.3, .1, 0, 3},
         {{"x", 0, "m"}, {"Delta_x", .3, "m"}, {"v", .1, "m/s"}, {"Weg", .3, "m"}},
         "ordinary decimal cancellation remains usable");
  expect("KIN_A_X", {-9007199254740992., 9007199254740992., 1, 1},
         {{"a", 2, "m/s^2"}, {"v", 9007199254740994., "m/s"}},
         "inverse acceleration retains position residual", 1e-14, 0);
  expect("KIN_V0_X", {-9007199254740992., 18014398509481984., 1, 1},
         {{"v0", 1, "m/s"}, {"v", 18014398509481984., "m/s"}},
         "inverse initial velocity retains position residual", 1e-14, 0);
  const double tiny = std::numeric_limits<double>::denorm_min();
  reject("KIN_STATE", {0, tiny, 0, .25}, "nonzero final displacement underflows");
  reject("KIN_T_V", {0, 4, tiny}, "nonzero final time underflows");
  reject("KIN_A_V", {0, tiny, 4}, "nonzero final acceleration underflows");
  for (double value : {1., .25, .9999999157063031, 1.000000084293697}) {
    for (unsigned digits : {10u, 15u}) {
      char expected[64]{};
      std::snprintf(expected, sizeof(expected), "%.*g", static_cast<int>(digits), value);
      need(formatNumber(value, digits) == expected, "precise root formatter matches host reference");
    }
  }
}

void syntheticSweep() {
  // Binary-exact independent trajectories, not results from calculateKinematics.
  // Modular loops vary signs, offsets, accelerations, times and length scales.
  for (double scale : {1.0 / 1024, 1.0, 1024.0}) {
    for (int i = 0; i < 240; ++i) {
      const double x0 = ((37 * i) % 101 - 50) * 1.25 * scale;
      const double v0 = ((19 * i) % 47 - 23) * .75 * scale;
      const double a = ((11 * i) % 31 - 15) * .25 * scale;
      const double t = (1 + (7 * i) % 23) * .125;
      const long double displacement =
          static_cast<long double>(v0) * t + static_cast<long double>(a) * t * t / 2;
      const double dx = static_cast<double>(displacement);
      const double x = static_cast<double>(static_cast<long double>(x0) + displacement);
      const double v = static_cast<double>(static_cast<long double>(v0) +
                                           static_cast<long double>(a) * t);
      long double path = (std::abs(static_cast<long double>(v0)) +
                          std::abs(static_cast<long double>(v))) * t / 2;
      if ((v0 < 0 && v > 0) || (v0 > 0 && v < 0)) {
        const long double stopTime = -static_cast<long double>(v0) / a;
        path = std::abs(static_cast<long double>(v0)) * stopTime / 2 +
               std::abs(static_cast<long double>(v)) * (t - stopTime) / 2;
      }
      const std::string label = "synthetic " + std::to_string(i) +
                                " scale " + formatNumber(scale);
      expect("KIN_STATE", {x0, v0, a, t},
             {{"x", x, "m"}, {"Delta_x", dx, "m"},
              {"v", v, "m/s"}, {"Weg", static_cast<double>(path), "m"}}, label);
      expect("KIN_A_V", {v0, v, t},
             {{"a", a, "m/s^2"}, {"Delta_x", dx, "m"}}, label);
      expect("KIN_A_X", {x0, v0, t, x},
             {{"a", a, "m/s^2"}, {"v", v, "m/s"}}, label);
      expect("KIN_V0_X", {x0, a, t, x},
             {{"v0", v0, "m/s"}, {"v", v, "m/s"}}, label);
      if (a != 0)
        expect("KIN_T_V", {v0, a, v},
               {{"t", t, "s"}, {"Delta_x", dx, "m"}}, label);
      else
        reject("KIN_T_V", {v0, a, v}, label + ": uniform velocity time nonunique");
      if (dx != 0 && v0 + v != 0)
        expect("KIN_A_DX", {v0, v, dx},
               {{"a", a, "m/s^2"}, {"t", t, "s"}}, label);
      else
        reject("KIN_A_DX", {v0, v, dx}, label + ": dx inverse nonunique");

      if (a == 0) {
        if (v0 == 0)
          reject("KIN_T_X", {x0, v0, a, x}, label + ": stationary time nonunique");
        else
          expect("KIN_T_X", {x0, v0, a, x},
                 {{"t", t, "s"}, {"v", v, "m/s"}}, label);
        continue;
      }
      // Viete's sum gives the other root from the independently chosen t.
      // This avoids duplicating the implementation's quadratic formula.
      const long double other = -2 * static_cast<long double>(v0) / a - t;
      if (other < 0 || v == 0) {
        expect("KIN_T_X", {x0, v0, a, x},
               {{"t", t, "s"}, {"v", v, "m/s"}}, label);
      } else {
        const double first = std::min(t, static_cast<double>(other));
        const double second = std::max(t, static_cast<double>(other));
        const double firstV = static_cast<double>(static_cast<long double>(v0) +
                                                  static_cast<long double>(a) * first);
        const double secondV = static_cast<double>(static_cast<long double>(v0) +
                                                   static_cast<long double>(a) * second);
        expect("KIN_T_X", {x0, v0, a, x},
               {{"t_1", first, "s"}, {"v_1", firstV, "m/s"},
                {"t_2", second, "s"}, {"v_2", secondV, "m/s"}}, label);
      }
    }
  }
}
} // namespace

int main() {
  catalog();
  unitChoices();
  originals();
  signedMotionAndRoots();
  invalidAndExtreme();
  reviewRegressions();
  syntheticSweep();
  std::cout << "PASS KINEMATICS: " << checks
            << " checks; 7 masks; 4 original scenarios\n";
}

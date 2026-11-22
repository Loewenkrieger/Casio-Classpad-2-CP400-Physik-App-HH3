// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "store.hpp"
#include <utility>

namespace physik {
inline constexpr double planck=6.62607015e-34,charge=1.602176634e-19,
  light=299792458.0,electronMass=9.1093837139e-31,bohrRadius=5.29177210544e-11,
  pi=3.141592653589793;
struct Parameter {std::string name,unit,defaultUnit;};
struct Calculation {std::string title,formula,name,unit,id;std::vector<Parameter> parameters;};
struct Unit {std::string name;double scale;};
struct Fit {double h=0,work=0,threshold=0,rms=0;};
bool calculationRecord(const Value& value,Calculation& result,std::string& error);
std::vector<Unit> unitOptions(const std::string& unit);
bool parseNumber(std::string text,double scale,double& result,std::string& error);
std::string formatNumber(double value);
std::string formatNumber(double value,unsigned significantDigits);
std::string resultText(const std::string& name,double value,const std::string& unit);
bool calculate(const Calculation& calculation,const std::vector<double>& values,double& result,std::string& error);
bool fitPlanck(const std::vector<std::pair<double,double>>& points,bool wavelength,Fit& result,std::string& error);
}

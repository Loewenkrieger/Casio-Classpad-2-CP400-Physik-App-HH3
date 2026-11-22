// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "calculator.hpp"

namespace physik {
struct KinematicValue {std::string name;double value;std::string unit;};
struct KinematicResult {std::vector<KinematicValue> values;std::string note;};
const std::vector<Calculation>& kinematicsCatalog();
bool isKinematic(const std::string& id);
std::vector<Unit> kinematicsUnits(const Parameter& parameter);
bool calculateKinematics(const Calculation& calc,const std::vector<double>& inputs,
                         KinematicResult& result,std::string& error);
}

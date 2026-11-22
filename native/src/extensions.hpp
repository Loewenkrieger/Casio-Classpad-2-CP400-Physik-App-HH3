// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "calculator.hpp"

namespace physik {
struct NativeValue { std::string name; double value; std::string unit; };
struct Plot {
  std::string title,xLabel,xUnit,yLabel,yUnit;
  std::vector<std::pair<double,double>> points,model;
  bool joined=true;
};
struct NativeResult {
  std::vector<NativeValue> values;
  std::string note;
  std::vector<Plot> plots;
};
struct NativeCategory { std::string id,title; };
struct Workflow {
  Calculation calculation;
  std::vector<std::string> defaults;
  unsigned minCount=0,maxCount=0;
  std::vector<Parameter> repeated;
  std::vector<std::string> repeatedDefaults;
};
std::vector<NativeCategory> nativeCategories();
std::vector<Workflow> nativeCatalog(const std::string& category);
bool nativeWorkflow(const std::string& id,Workflow& result);
std::vector<Unit> nativeUnits(const Parameter& parameter);
bool nativeCalculate(const Workflow& workflow,const std::vector<double>& values,
                     NativeResult& result,std::string& error);
bool nativeValidate(const Workflow& workflow,const std::vector<double>& values,std::string& error);
bool nativeFinish(NativeResult& result,std::string& error);
void expandWorkflow(const Workflow& workflow,unsigned count,std::vector<Parameter>& parameters,
                    std::vector<std::string>& defaults);

std::vector<NativeCategory> mechanicsCategories();
std::vector<Workflow> mechanicsCatalog(const std::string& category);
bool mechanicsCalculate(const std::string& id,const std::vector<double>& values,NativeResult& result,std::string& error);
std::vector<NativeCategory> fieldsCategories();
std::vector<Workflow> fieldsCatalog(const std::string& category);
bool fieldsCalculate(const std::string& id,const std::vector<double>& values,NativeResult& result,std::string& error);
std::vector<NativeCategory> opticsCategories();
std::vector<Workflow> opticsCatalog(const std::string& category);
bool opticsCalculate(const std::string& id,const std::vector<double>& values,NativeResult& result,std::string& error);
std::vector<NativeCategory> analysisCategories();
std::vector<Workflow> analysisCatalog(const std::string& category);
bool analysisCalculate(const std::string& id,const std::vector<double>& values,NativeResult& result,std::string& error);
bool planckAnalysis(const std::vector<std::pair<double,double>>& points,bool wavelength,NativeResult& result,std::string& error);
std::vector<NativeCategory> simulationCategories();
std::vector<Workflow> simulationCatalog(const std::string& category);
bool simulationCalculate(const std::string& id,const std::vector<double>& values,NativeResult& result,std::string& error);
}

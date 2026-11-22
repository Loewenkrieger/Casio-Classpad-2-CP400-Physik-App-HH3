// SPDX-License-Identifier: GPL-3.0-or-later
#include "extensions.hpp"
#include "catalog_data.hpp"
#include <algorithm>
#include <cmath>

namespace physik {
std::vector<NativeCategory> nativeCategories(){
  std::vector<NativeCategory> result;
  for(auto fn:{mechanicsCategories,fieldsCategories,opticsCategories,analysisCategories,simulationCategories}){
    auto part=fn();result.insert(result.end(),part.begin(),part.end());
  }
  return result;
}
std::vector<Workflow> nativeCatalog(const std::string& category){
#ifdef PHYSIK_DATA_CATALOG
  return dataCatalog(category);
#else
  for(auto fn:{mechanicsCatalog,fieldsCatalog,opticsCatalog,analysisCatalog,simulationCatalog}){
    auto part=fn(category);if(!part.empty())return part;
  }
  return {};
#endif
}
bool nativeWorkflow(const std::string& id,Workflow& result){
#ifdef PHYSIK_DATA_CATALOG
  return dataWorkflow(id,result);
#else
  for(const auto& category:nativeCategories())for(const auto& workflow:nativeCatalog(category.id)){
    if(workflow.calculation.id==id){result=workflow;return true;}
  }
  result={};return false;
#endif
}
void expandWorkflow(const Workflow& workflow,unsigned count,std::vector<Parameter>& parameters,
                    std::vector<std::string>& defaults){
  parameters=workflow.calculation.parameters;defaults=workflow.defaults;
  defaults.resize(parameters.size());
  if(!workflow.maxCount)return;
  if(count<workflow.minCount||count>workflow.maxCount)return;
  for(unsigned i=0;i<count;++i)for(size_t j=0;j<workflow.repeated.size();++j){
    auto parameter=workflow.repeated[j];parameter.name=std::to_string(i+1)+": "+parameter.name;
    parameters.push_back(std::move(parameter));
    defaults.push_back(j<workflow.repeatedDefaults.size()?workflow.repeatedDefaults[j]:"");
  }
}
std::vector<Unit> nativeUnits(const Parameter& p){
  const auto& u=p.unit;
  if(u=="m")return {{"m",1},{"km",1000},{"cm",.01},{"mm",.001},{"um",1e-6},{"nm",1e-9},{"pm",1e-12}};
  if(u=="m^2")return {{"m^2",1},{"cm^2",1e-4},{"mm^2",1e-6}};
  if(u=="m^3")return {{"m^3",1},{"cm^3",1e-6},{"mm^3",1e-9},{"l",.001}};
  if(u=="s")return {{"s",1},{"min",60},{"h",3600},{"ms",.001},{"us",1e-6},{"ns",1e-9},{"ps",1e-12},{"fs",1e-15}};
  if(u=="m/s")return {{"m/s",1},{"km/h",1/3.6},{"cm/s",.01}};
  if(u=="m/s^2")return {{"m/s^2",1},{"g",9.81}};
  if(u=="kg")return {{"kg",1},{"g",.001},{"mg",1e-6},{"ug",1e-9},{"u",1.66053906892e-27}};
  if(u=="rad")return {{"rad",1},{"deg",pi/180}};
  if(u=="Hz")return {{"Hz",1},{"kHz",1e3},{"MHz",1e6},{"GHz",1e9},{"THz",1e12}};
  if(u=="V")return {{"V",1},{"mV",.001},{"kV",1000}};
  if(u=="A")return {{"A",1},{"mA",.001},{"uA",1e-6},{"nA",1e-9}};
  if(u=="C")return {{"C",1},{"mC",.001},{"uC",1e-6},{"nC",1e-9},{"pC",1e-12},{"e",charge}};
  if(u=="F")return {{"F",1},{"mF",.001},{"uF",1e-6},{"nF",1e-9},{"pF",1e-12}};
  if(u=="H")return {{"H",1},{"mH",.001},{"uH",1e-6}};
  if(u=="Ohm"||u=="ohm")return {{u,1},{"kOhm",1000},{"MOhm",1e6},{"mOhm",.001}};
  if(u=="T")return {{"T",1},{"mT",.001},{"uT",1e-6}};
  if(u=="N")return {{"N",1},{"kN",1000},{"mN",.001},{"uN",1e-6},{"nN",1e-9}};
  if(u=="J")return {{"J",1},{"kJ",1000},{"mJ",.001},{"eV",charge},{"keV",charge*1000},{"MeV",charge*1e6}};
  if(u=="W")return {{"W",1},{"kW",1000},{"mW",.001}};
  if(u=="N/C"||u=="V/m")return {{u,1},{"kV/m",1000},{"V/cm",100}};
  if(u=="kg/m^3")return {{"kg/m^3",1},{"g/cm^3",1000}};
  if(u=="Ohm*m")return {{"Ohm*m",1},{"Ohm*mm^2/m",1e-6}};
  if(u=="Wb")return {{"Wb",1},{"mWb",.001},{"uWb",1e-6}};
  return unitOptions(u);
}
bool nativeValidate(const Workflow& w,const std::vector<double>& v,std::string& error){
  error.clear();size_t expected=w.calculation.parameters.size();
  if(w.maxCount){
    if(v.empty()||!std::isfinite(v[0])||v[0]!=std::floor(v[0])||v[0]<w.minCount||v[0]>w.maxCount){
      error="Anzahl ausserhalb des erlaubten Bereichs";return false;
    }
    expected+=static_cast<size_t>(v[0])*w.repeated.size();
  }
  if(v.size()!=expected){error="Falsche Anzahl Eingaben";return false;}
  for(double x:v)if(!std::isfinite(x)){error="Nicht endliche Eingabe";return false;}
  return true;
}
bool nativeFinish(NativeResult& r,std::string& error){
  bool valid=r.values.size()<=512&&r.plots.size()<=12;
  for(const auto& v:r.values)valid=valid&&std::isfinite(v.value);
  for(const auto& p:r.plots){
    valid=valid&&p.points.size()<=400&&p.model.size()<=400;
    for(const auto& series:{&p.points,&p.model})for(auto [x,y]:*series)valid=valid&&std::isfinite(x)&&std::isfinite(y);
  }
  if(!valid){r={};error="Ergebnis ausserhalb des Zahlen- oder Speicherbereichs";return false;}
  return true;
}
bool nativeCalculate(const Workflow& w,const std::vector<double>& v,NativeResult& r,std::string& error){
  r={};if(!nativeValidate(w,v,error))return false;
  const auto& id=w.calculation.id;bool ok=false;
  if(id.rfind("M_",0)==0)ok=mechanicsCalculate(id,v,r,error);
  else if(id.rfind("E_",0)==0)ok=fieldsCalculate(id,v,r,error);
  else if(id.rfind("O_",0)==0)ok=opticsCalculate(id,v,r,error);
  else if(id.rfind("A_",0)==0)ok=analysisCalculate(id,v,r,error);
  else if(id.rfind("S_",0)==0)ok=simulationCalculate(id,v,r,error);
  else error="Unbekannter nativer Rechner";
  if(!ok){r={};if(error.empty())error="Rechnung nicht moeglich";return false;}
  return nativeFinish(r,error);
}
}

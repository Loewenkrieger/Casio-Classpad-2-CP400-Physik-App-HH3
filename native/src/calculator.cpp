// SPDX-License-Identifier: GPL-3.0-or-later
#include "calculator.hpp"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <limits>

namespace physik {
static_assert(sizeof(double)==8&&std::numeric_limits<double>::digits>=53,"Physics requires binary64");
namespace {
bool text(const Value& value){return value.type==Value::Text;}
struct Engine {
  std::string error;
  static constexpr double PI=pi,H_PLANCK=planck,E_CHARGE=charge,LIGHT_C=light,ELECTRON_MASS=electronMass,BOHR_A=bohrRadius;
  double fail(const std::string& message){if(error.empty())error=message;return 0;}
  double finite(double value){if(!std::isfinite(value))return fail("Zahl nicht endlich / zu gross");return value;}
  double positive(double value){if(!std::isfinite(value)||value<=0){fail("Wert muss groesser als 0 sein");return 1;}return value;}
  double nonnegative(double value){if(!std::isfinite(value)||value<0)return fail("Wert darf nicht negativ sein");return value;}
  double natural(double value){value=positive(value);if(value!=std::floor(value)){fail("Quantenzahl muss ganzzahlig sein");return 1;}return value;}
  double need_nonzero(double value,const std::string& name){if(value==0){fail(name+" darf nicht 0 sein");return 1;}return value;}
  double need_sqrt(double value){if(value<0){fail("Wurzel aus negativer Zahl");return 1;}return std::sqrt(value);}
  double classical_v(double value){if(value>=LIGHT_C*.1)return fail("v >= 0.1 c: relativistisch rechnen");return value;}
  double box_energy(double a,double n,double mass=ELECTRON_MASS){double p=H_PLANCK*natural(n)/(2*positive(a));return (p/positive(mass))*p/2;}
#include "equations.inc"
};
}
bool calculationRecord(const Value& value,Calculation& result,std::string& error){
  error.clear();
  if(value.type!=Value::Tuple||value.items.size()!=6){error="Ungueltiger Rechnerdatensatz";return false;}
  const auto& v=value.items;
  for(int i:{0,1,2,3,5})if(!text(v[i])){error="Ungueltiger Rechnertext";return false;}
  if(v[4].type!=Value::Tuple||v[4].items.size()>4){error="Ungueltige Eingaben";return false;}
  result={v[0].text,v[1].text,v[2].text,v[3].text,v[5].text,{}};
  for(const auto& p:v[4].items){
    if(p.type!=Value::Tuple||(p.items.size()!=2&&p.items.size()!=3)||!text(p.items[0])||!text(p.items[1])||(p.items.size()==3&&!text(p.items[2]))){error="Ungueltiger Eingabeparameter";return false;}
    result.parameters.push_back({p.items[0].text,p.items[1].text,p.items.size()==3?p.items[2].text:p.items[1].text});
  }
  return true;
}
std::vector<Unit> unitOptions(const std::string& unit){
  if(unit=="m")return {{"m",1},{"nm",1e-9},{"pm",1e-12},{"um",1e-6},{"mm",1e-3}};
  if(unit=="J")return {{"J",1},{"eV",charge},{"keV",charge*1000}};
  if(unit=="V")return {{"V",1},{"kV",1000}};
  if(unit=="s")return {{"s",1},{"ns",1e-9},{"ps",1e-12},{"fs",1e-15}};
  if(unit=="Hz")return {{"Hz",1},{"THz",1e12}};
  return {{unit,1}};
}
bool parseNumber(std::string value,double scale,double& result,std::string& error){
  error.clear();result=0;
  if(value.empty()||value.size()>96){error="Bitte eine Zahl eingeben";return false;}
  for(char& c:value){if(c==',')c='.';if(std::string("0123456789.+-eE").find(c)==std::string::npos){error="Ungueltige Zahl";return false;}}
  char* end=nullptr;const double raw=std::strtod(value.c_str(),&end);
  if(end==value.c_str()||*end||!std::isfinite(raw)||!std::isfinite(scale)||scale<=0){error="Ungueltige oder zu grosse Zahl";return false;}
  bool nonzero=false;for(char c:value){if(c=='e'||c=='E')break;if(c>='1'&&c<='9')nonzero=true;}
  result=raw*scale;
  if(!std::isfinite(result)){error="Zahl nach Umrechnung zu gross";return false;}
  if((raw==0&&nonzero)||(raw!=0&&result==0)){error="Wert zu klein fuer diese Laufzeit";return false;}
  return true;
}
std::string formatNumber(double value){
  return formatNumber(value,6);
}
std::string formatNumber(double value,unsigned significantDigits){
  if(!std::isfinite(value))return "[Zahlenbereich ueberschritten]";
  if(value==0)return "0";
  significantDigits=std::clamp(significantDigits,1u,15u);
  const bool negative=value<0;double scaled=std::abs(value);int exponent=0;
  // Normalize in bounded steps, including subnormal values, without printf/newlib.
  while(scaled>=1e100){scaled/=1e100;exponent+=100;}
  while(scaled>=10){scaled/=10;++exponent;}
  while(scaled<1e-99){scaled*=1e100;exponent-=100;}
  while(scaled<1){scaled*=10;--exponent;}
  uint64_t limit=1;for(unsigned i=0;i<significantDigits;++i)limit*=10;
  uint64_t significant=static_cast<uint64_t>(std::floor(scaled*static_cast<double>(limit/10)+.5));
  if(significant>=limit){significant/=10;++exponent;}
  std::string digits=std::to_string(significant);while(digits.size()<significantDigits)digits="0"+digits;
  const bool scientific=exponent< -4||exponent>=static_cast<int>(significantDigits);
  const int point=scientific?1:exponent+1;
  std::string result=point<=0?"0."+std::string(-point,'0')+digits:digits.substr(0,point)+"."+digits.substr(point);
  while(result.back()=='0')result.pop_back();
  if(result.back()=='.')result.pop_back();
  if(scientific){std::string power=std::to_string(std::abs(exponent));if(power.size()<2)power="0"+power;
    result+="e";result+=exponent<0?"-":"+";result+=power;}
  return (negative?"-":"")+result;
}
std::string resultText(const std::string& name,double value,const std::string& unit){
  std::string result=name+" = "+formatNumber(value)+" "+unit;
  if(unit=="J")result+="\n= "+formatNumber(value/charge)+" eV";
  else if(unit=="m"&&value!=0&&std::abs(value)<1e-6){const bool pm=std::abs(value)<1e-9;result+="\n= "+formatNumber(value/(pm?1e-12:1e-9))+(pm?" pm":" nm");}
  else if(unit=="V"&&std::abs(value)>=1000)result+="\n= "+formatNumber(value/1000)+" kV";
  return result;
}
bool calculate(const Calculation& calc,const std::vector<double>& values,double& result,std::string& error){
  error.clear();result=0;
  if(values.size()!=calc.parameters.size()||values.empty()||calc.id.rfind("FIT_",0)==0){error="Falsche Anzahl Eingaben";return false;}
  for(double value:values)if(!std::isfinite(value)){error="Zahl nicht endlich / zu gross";return false;}
  Engine engine;
  if(engine.expectedCount(calc.id)!=static_cast<int>(values.size())){error="Unbekannte Formel oder unpassende Eingaben";return false;}
  result=engine.finite(engine.classic(calc.id,values));error=engine.error;return error.empty();
}
bool fitPlanck(const std::vector<std::pair<double,double>>& points,bool wavelength,Fit& result,std::string& error){
  error.clear();result={};
  if(points.size()<2||points.size()>10){error="2 bis 10 Messpunkte erforderlich";return false;}
  std::vector<double> x,y;double xm=0,ym=0;
  for(auto [a,b]:points){if(!std::isfinite(a)||!std::isfinite(b)||a<=0||b<0){error="Messwerte: x > 0 und y >= 0";return false;}
    const double xx=(wavelength?light/a:a)/1e14,yy=wavelength?b/charge:b;
    if(!std::isfinite(xx)||!std::isfinite(yy)){error="Messwerte ausserhalb des Zahlenbereichs";return false;}
    x.push_back(xx);y.push_back(yy);xm+=xx;ym+=yy;
  }
  xm/=points.size();ym/=points.size();double spread=0,cross=0;
  for(size_t i=0;i<x.size();++i){spread+=(x[i]-xm)*(x[i]-xm);cross+=(x[i]-xm)*(y[i]-ym);}
  if(!std::isfinite(spread)||spread<=0){error="Frequenzen muessen verschieden sein";return false;}
  const double slope=cross/spread,intercept=ym-slope*xm;
  if(!std::isfinite(slope)||slope<=0||intercept>0){error="Fit ergibt unphysikalische Steigung oder Austrittsarbeit";return false;}
  double residual=0;for(size_t i=0;i<x.size();++i){const double delta=y[i]-slope*x[i]-intercept;residual+=delta*delta;}
  result={slope*charge/1e14,-intercept*charge,0,std::sqrt(residual/x.size())};result.threshold=result.work/result.h;
  if(!std::isfinite(result.h)||result.h<=0||!std::isfinite(result.work)||!std::isfinite(result.threshold)||!std::isfinite(result.rms)){error="Fit ausserhalb des Zahlenbereichs";return false;}
  return true;
}
}

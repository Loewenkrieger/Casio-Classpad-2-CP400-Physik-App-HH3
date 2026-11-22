// SPDX-License-Identifier: GPL-3.0-or-later
// Standalone host test: link src/optics.cpp and src/calculator.cpp, not extensions.cpp.
#include "extensions.hpp"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <map>
#include <set>

namespace {
using namespace physik;
unsigned checks=0;
std::map<std::string,Workflow> catalog;
std::map<std::string,std::vector<double>> examples;
constexpr double degrees=pi/180;
void need(bool condition,const std::string& message){
  ++checks;
  if(!condition){std::cerr<<"FAIL OPTICS: "<<message<<'\n';std::exit(1);}
}
void equal(double actual,double expected,const std::string& message,double relative=2e-11,double absolute=0){
  need(std::isfinite(actual)&&std::isfinite(expected)&&
       std::abs(actual-expected)<=std::max(absolute,relative*std::abs(expected)),
       message+" expected "+formatNumber(expected,14)+", got "+formatNumber(actual,14));
}
void clean(const NativeResult& r,const std::string& context){
  need(!r.values.empty(),context+" named results");
  std::set<std::string> names;
  for(const auto& v:r.values){
    need(!v.name.empty()&&names.insert(v.name).second,context+" unique nonempty name");
    need(std::isfinite(v.value),context+" finite value");
  }
  for(const auto& p:r.plots){
    need(!p.title.empty()&&!p.xLabel.empty()&&!p.yLabel.empty(),context+" plot labels");
    need(p.points.size()<=400&&p.model.size()<=400,context+" plot bounds");
    for(const auto* series:{&p.points,&p.model})for(auto [x,y]:*series)
      need(std::isfinite(x)&&std::isfinite(y),context+" finite plot point");
  }
}
NativeResult dirty(){return {{{"stale",123,"bad"}},"stale",{{"stale","x","","y","",{{0,0}},{},true}}};}
NativeResult solve(const std::string& id,const std::vector<double>& v){
  NativeResult r=dirty();std::string error="stale";
  const bool ok=opticsCalculate(id,v,r,error);
  need(ok,id+": "+error);need(error.empty(),id+" clears old error");clean(r,id);return r;
}
double value(const NativeResult& r,const std::string& name,const std::string& unit){
  for(const auto& v:r.values)if(v.name==name){need(v.unit==unit,name+" unit "+unit);return v.value;}
  need(false,"missing result "+name);return 0;
}
void expect(const std::string& id,const std::vector<double>& v,const std::string& name,
            double expected,const std::string& unit="",double relative=2e-11){
  equal(value(solve(id,v),name,unit),expected,id+" "+name,relative);
}
void reject(const std::string& id,const std::vector<double>& v){
  NativeResult r=dirty();std::string error;
  need(!opticsCalculate(id,v,r,error),id+" must reject");
  need(!error.empty(),id+" German failure explanation");
  need(r.values.empty()&&r.plots.empty()&&r.note.empty(),id+" clears all stale/partial results");
}
double input(const std::string& text,double scale){
  double result=0;std::string error;
  need(parseNumber(text,scale,result,error),"parse SI input "+text+": "+error);return result;
}
double defaultScale(const Parameter& p){
  if(p.defaultUnit==p.unit)return 1;
  const std::map<std::string,double> scales={{"deg",degrees},{"nm",1e-9},{"pm",1e-12},
    {"um",1e-6},{"mm",1e-3},{"cm",1e-2},{"kV",1e3}};
  const auto i=scales.find(p.defaultUnit);need(i!=scales.end(),"known default unit "+p.defaultUnit);return i->second;
}
void catalogue(){
  const std::map<std::string,size_t> counts={{"O_DG",16},{"O_WAVE",9},{"O_REFRACT",9},
    {"O_POLAR",5},{"O_FILM",8},{"O_SLIT",13},{"O_RESOLVE",7},{"O_QUANTUM",4}};
  const auto categories=opticsCategories();need(categories.size()==8,"eight categories");
  std::set<std::string> categoryIds;
  unsigned variable=0;
  for(const auto& c:categories){
    need(counts.count(c.id)==1&&categoryIds.insert(c.id).second&&!c.title.empty(),"valid category");
    const auto workflows=opticsCatalog(c.id);need(workflows.size()==counts.at(c.id),c.id+" exact count");
    for(const auto& w:workflows){
      const auto& calc=w.calculation;
      need(calc.id.rfind("O_",0)==0&&catalog.emplace(calc.id,w).second,"unique O_ ID");
      need(!calc.title.empty()&&!calc.formula.empty()&&!calc.name.empty(),calc.id+" metadata");
      need(w.defaults.size()==calc.parameters.size(),calc.id+" base defaults");
      need(w.repeated.size()==w.repeatedDefaults.size(),calc.id+" repeat defaults");
      need(w.minCount<=w.maxCount&&w.maxCount<=40,calc.id+" bounded repeat count");
      std::vector<double> v;
      for(size_t i=0;i<calc.parameters.size();++i){
        const auto& p=calc.parameters[i];need(!p.name.empty(),calc.id+" parameter name");
        if(p.unit=="rad")need(p.defaultUnit=="deg",calc.id+" SI radians / displayed degrees");
        v.push_back(input(w.defaults[i],defaultScale(p)));
      }
      if(w.maxCount){
        ++variable;need(w.minCount==1&&w.maxCount==8&&w.repeated.size()==1,calc.id+" filter contract");
        need(calc.parameters.front().name.find("Filterzahl")!=std::string::npos,calc.id+" count first");
        need(w.repeated[0].unit=="rad"&&w.repeated[0].defaultUnit=="deg",calc.id+" repeated angle SI");
        for(unsigned j=0;j<static_cast<unsigned>(v[0]);++j)
          v.push_back(input(w.repeatedDefaults[0],defaultScale(w.repeated[0])));
      }else need(w.minCount==0&&w.repeated.empty(),calc.id+" fixed workflow");
      solve(calc.id,v);examples[calc.id]=v;
    }
  }
  need(catalog.size()==71&&variable==2,"71 workflows: 69 fixed, 2 dynamic");
  need(opticsCatalog("").empty()&&opticsCatalog("O_UNKNOWN").empty(),"unknown category empty");
}
void apiValidation(){
  const double inf=std::numeric_limits<double>::infinity(),nan=std::numeric_limits<double>::quiet_NaN();
  for(const auto& [id,v]:examples){
    reject(id,{});
    auto shortInput=v;shortInput.pop_back();reject(id,shortInput);
    auto longInput=v;longInput.push_back(0);reject(id,longInput);
    for(size_t i=0;i<v.size();++i)for(double invalid:{inf,-inf,nan}){
      auto a=v;a[i]=invalid;reject(id,a);
    }
    // Every public workflow also has to survive finite, adversarial magnitudes.
    for(size_t i=0;i<v.size();++i)for(double edge:{0.0,-1.0,1e-300,1e300,std::numeric_limits<double>::max()}){
      auto a=v;a[i]=edge;NativeResult r=dirty();std::string error="stale";
      const bool ok=opticsCalculate(id,a,r,error);
      if(ok){need(error.empty(),id+" finite edge success clears error");clean(r,id);}
      else {need(!error.empty(),id+" edge error");need(r.values.empty()&&r.plots.empty()&&r.note.empty(),id+" edge result reset");}
    }
  }
  reject("",{});reject("O_DG_ANGLE_SUFFIX",{1,2,1});reject("Q_DS",{1,2,3});
}
void diffraction(){
  // A 3-4-5 ray supplies independent exact geometry, not an implementation round trip.
  const double wavelength=600e-9,d=1e-6,L=4,xx=3,a=std::atan2(3.0,4.0);
  expect("O_DG_ANGLE",{wavelength,d,1},"theta",a,"rad");
  expect("O_DG_LAMBDA",{d,1,a},"lambda",wavelength,"m");
  expect("O_DG_D",{wavelength,1,a},"d",d,"m");
  expect("O_DG_ORDER",{wavelength,d,a},"k",1);
  for(const std::string suffix:{"SCREEN","DISTANCE"}){
    const double measured=suffix=="SCREEN"?xx:2*xx;
    const auto forward=solve("O_DG_"+suffix,{wavelength,d,1,L});
    equal(value(forward,"x einseitig","m"),xx,"one-sided x");
    equal(value(forward,"D symmetrisch","m"),2*xx,"symmetric D");
    expect("O_DG_LAMBDA_"+suffix,{d,1,L,measured},"lambda",wavelength,"m");
    expect("O_DG_D_"+suffix,{wavelength,1,L,measured},"d",d,"m");
    expect("O_DG_ORDER_"+suffix,{wavelength,d,L,measured},"k",1);
    expect("O_DG_L_"+suffix,{wavelength,d,1,measured},"L",L,"m");
  }
  expect("O_DG_ANGLE",{1,3,0},"theta",0,"rad");
  expect("O_DG_SCREEN",{1,3,0,2},"x einseitig",0,"m");
  expect("O_DG_ORDER",{1,3,0},"k",0);
  expect("O_DG_ORDER_SCREEN",{1,3,2,0},"k",0);
  const auto grazing=solve("O_DG_ANGLE",{1,3,3});
  equal(value(grazing,"theta","rad"),pi/2,"grazing angle");
  need(grazing.note.find("keine endliche")!=std::string::npos,"grazing note");
  expect("O_DG_LAMBDA",{3,3,pi/2},"lambda",1,"m");
  expect("O_DG_D",{1,3,pi/2},"d",3,"m");
  expect("O_DG_ORDER",{1,3,pi/2},"k",3);
  expect("O_DG_MAX",{1,3},"k_max propagierend",3);
  expect("O_DG_MAX",{1,3},"k_max endlicher Schirm",2);
  expect("O_DG_MAX",{1,3},"Maxima gesamt inkl. Mitte",7);
  expect("O_DG_MAX",{1,3},"Maxima auf endlichem Schirm",5);
  expect("O_DG_MAX",{600e-9,5*600e-9},"k_max endlicher Schirm",4);
  expect("O_DG_MAX",{1,std::nextafter(3.0,0.0)},"k_max propagierend",2);
  expect("O_DG_MAX",{1,std::nextafter(3.0,4.0)},"k_max endlicher Schirm",3);
  expect("O_DG_MAX",{2,1},"Maxima gesamt inkl. Mitte",1);
  expect("O_DG_MAX",{2,1},"Maxima auf endlichem Schirm",1);
  for(const auto& [id,v]:std::vector<std::pair<std::string,std::vector<double>>>{
      {"O_DG_SCREEN",{1,3,3,1}},{"O_DG_DISTANCE",{1,3,3,1}},
      {"O_DG_L_SCREEN",{1,3,3,1}},{"O_DG_ANGLE",{1,3,4}},
      {"O_DG_ANGLE",{1,3,1.5}},{"O_DG_ANGLE",{1,3,-1}},
      {"O_DG_LAMBDA",{1,0,a}},{"O_DG_LAMBDA",{1,1,0}},
      {"O_DG_D",{1,1,0}},{"O_DG_D",{1,1,-a}},
      {"O_DG_ORDER",{wavelength,d,std::asin(.65)}},{"O_DG_ORDER",{1,3,1e-20}},
      {"O_DG_ORDER",{1,3,pi/2+1e-9}},
      {"O_DG_L_SCREEN",{1,3,0,1}},{"O_DG_L_SCREEN",{1,3,1,0}},
      {"O_DG_LAMBDA_SCREEN",{1,1,1,0}},{"O_DG_D_SCREEN",{1,1,0,1}},
      {"O_DG_ORDER_SCREEN",{1,3,1,-1}},{"O_DG_SEPARATION",{1,1.5,3,2,1}},
      {"O_DG_MAX",{1e-300,1e300}},{"O_DG_MAX",{1e300,1e-300}}})reject(id,v);
  // Multiple physically unrelated scales exercise both large angles and SI lengths.
  for(double scale:{1e-6,1.0,1e6})for(unsigned order=1;order<=6;++order){
    const double lambda=.05*scale,spacing=scale,thetaIndependent=std::asin(.05*order);
    expect("O_DG_ANGLE",{lambda,spacing,static_cast<double>(order)},"theta",thetaIndependent,"rad");
    expect("O_DG_ORDER",{lambda,spacing,thetaIndependent},"k",order);
    expect("O_DG_LAMBDA",{spacing,static_cast<double>(order),thetaIndependent},"lambda",lambda,"m");
  }
}
void standingWaves(){
  for(const std::string boundary:{"FF","OO","CO"})for(unsigned n=1;n<=7;++n){
    const double harmonic=boundary=="CO"?2*n-1:n,denominator=boundary=="CO"?4:2;
    const double L=.65,speed=572,frequency=harmonic*speed/(denominator*L),mode=n;
    const std::string prefix="O_"+boundary+"_";
    const auto r=solve(prefix+"F",{L,speed,mode});
    equal(value(r,"f","Hz"),frequency,prefix+"frequency");
    equal(value(r,"lambda","m"),denominator*L/harmonic,prefix+"mode wavelength");
    equal(value(r,"Harmonische",""),harmonic,prefix+"harmonic vs mode");
    expect(prefix+"L",{frequency,speed,mode},"L",L,"m");
    expect(prefix+"V",{L,frequency,mode},"v",speed,"m/s");
    reject(prefix+"F",{L,speed,0});reject(prefix+"L",{frequency,speed,1.5});reject(prefix+"V",{-L,frequency,mode});
  }
  expect("O_FF_V",{.65,440,1},"v",572,"m/s");
  expect("O_OO_F",{.5,340,1},"f",340,"Hz");
  expect("O_CO_F",{.5,340,1},"f",170,"Hz");
  expect("O_CO_F",{.5,340,2},"f",510,"Hz");
}
void refraction(){
  const double a=30*degrees,b=std::asin(1.0/3),critical=std::asin(2.0/3),brewster=std::atan(1.5);
  expect("O_SNELL_BETA",{1,1.5,a},"beta",b,"rad");
  expect("O_SNELL_ALPHA",{1,1.5,b},"alpha",a,"rad");
  expect("O_SNELL_N1",{1.5,a,b},"n1",1);
  expect("O_SNELL_N2",{1,a,b},"n2",1.5);
  expect("O_SNELL_BETA",{1,1.5,0},"beta",0,"rad");
  expect("O_SNELL_ALPHA",{1,1.5,0},"alpha",0,"rad");
  expect("O_CRITICAL",{1.5,1},"alpha kritisch",critical,"rad");
  expect("O_SNELL_ALPHA",{1.5,1,pi/2},"alpha",critical,"rad");
  expect("O_SNELL_BETA",{1.5,1,critical},"beta",pi/2,"rad");
  expect("O_TIR",{1.5,1,critical},"Totalreflexion",0);
  expect("O_TIR",{1.5,1,critical+1e-10},"Totalreflexion",1);
  expect("O_TIR",{1.5,1,critical-1e-10},"Totalreflexion",0);
  for(double higher:{1.1,1.33,1.5,1.7,2.4})for(double lower:{.9,1.0}){
    const double boundary=std::asin(lower/higher);
    expect("O_TIR",{higher,lower,boundary},"Totalreflexion",0);
    expect("O_SNELL_BETA",{higher,lower,boundary},"beta",pi/2,"rad");
    expect("O_TIR",{higher,lower,std::nextafter(boundary,pi/2)},"Totalreflexion",1);
    expect("O_TIR",{higher,lower,std::nextafter(boundary,0.0)},"Totalreflexion",0);
    reject("O_SNELL_BETA",{higher,lower,std::nextafter(boundary,pi/2)});
    reject("O_FILM_RMAX_D",{higher,lower,3,boundary,600e-9,1});
    reject("O_FILM_RMAX_D",{higher,3,lower,boundary,600e-9,1});
  }
  const auto total=solve("O_TIR",{1.5,1,pi/3});
  need(total.values.size()==2,"TIR has critical angle, no invented real beta");
  expect("O_TIR",{1,1.5,a},"Totalreflexion",0);
  expect("O_BREWSTER",{1,1.5},"alpha Brewster",brewster,"rad");
  expect("O_BREWSTER",{1,1.5},"beta",pi/2-brewster,"rad");
  expect("O_BREWSTER_N1",{1.5,brewster},"n1",1);
  expect("O_BREWSTER_N2",{1,brewster},"n2",1.5);
  for(const auto& [id,v]:std::vector<std::pair<std::string,std::vector<double>>>{
      {"O_SNELL_BETA",{1.5,1,pi/3}},{"O_SNELL_ALPHA",{1,1.5,pi/3}},
      {"O_SNELL_N1",{1.5,0,0}},{"O_SNELL_N2",{1,a,0}},
      {"O_SNELL_N1",{1.5,pi/2,b}},{"O_SNELL_BETA",{1,1.5,pi/2}},
      {"O_TIR",{1,1.5,pi/2}},{"O_CRITICAL",{1,1.5}},{"O_CRITICAL",{1,1}},
      {"O_BREWSTER",{1,1}},{"O_BREWSTER_N1",{1.5,0}},
      {"O_BREWSTER_N2",{1,pi/2}},{"O_BREWSTER_N1",{1.5,pi/4}},
      {"O_BREWSTER_N2",{1,pi/4}},{"O_SNELL_BETA",{-1,1.5,a}}})reject(id,v);
}
void polarization(){
  expect("O_MALUS",{8,pi/6},"I",6,"W/m^2");
  expect("O_MALUS",{8,-pi/6},"I",6,"W/m^2");
  expect("O_MALUS",{8,pi/2},"I",0,"W/m^2");
  expect("O_MALUS",{0,pi/4},"I",0,"W/m^2");
  expect("O_MALUS_ANGLE",{8,6},"alpha",pi/6,"rad");
  expect("O_MALUS_ANGLE",{8,0},"alpha",pi/2,"rad");
  expect("O_MALUS_ANGLE",{8,8},"alpha",0,"rad");
  expect("O_FIRST_FILTER",{8},"I",4,"W/m^2");
  expect("O_FILTERS_U",{2,8,0,pi/2},"I",0,"W/m^2");
  expect("O_FILTERS_U",{3,8,0,pi/4,pi/2},"I",1,"W/m^2");
  expect("O_FILTERS_P",{3,8,0,0,pi/4,pi/2},"I",2,"W/m^2");
  expect("O_FILTERS_P",{1,8,0,pi/2},"I",0,"W/m^2");
  expect("O_FILTERS_P",{1,8,-pi,pi},"I",8,"W/m^2");
  expect("O_FILTERS_U",{1,8,pi/3},"I",4,"W/m^2");
  for(unsigned n=1;n<=8;++n){
    std::vector<double> u={static_cast<double>(n),8};u.resize(n+2,0);
    const auto r=solve("O_FILTERS_U",u);equal(value(r,"I","W/m^2"),4,"unpolarized first filter only");
    need(r.plots.size()==1&&r.plots[0].points.size()==n+1,"bounded sequential plot");
    std::vector<double> p={static_cast<double>(n),8,0};p.resize(n+3,0);
    expect("O_FILTERS_P",p,"I",8,"W/m^2");
  }
  for(const auto& [id,v]:std::vector<std::pair<std::string,std::vector<double>>>{
      {"O_MALUS",{-1,0}},{"O_MALUS",{8,2*pi}},{"O_MALUS_ANGLE",{0,0}},
      {"O_MALUS_ANGLE",{8,9}},{"O_MALUS_ANGLE",{8,-1}},
      {"O_FILTERS_U",{0,8}},{"O_FILTERS_U",{1.5,8,0}},
      {"O_FILTERS_P",{1,8,0,4}},{"O_FILTERS_U",{9,8,0,0,0,0,0,0,0,0,0}},
      {"O_FILTERS_P",{2,8,0,0}},{"O_FILTERS_U",{1,8,0,0}}})reject(id,v);
}
void thinFilms(){
  // Independently check the full reflected phase, not the module's q expression.
  for(const auto& indices:std::vector<std::vector<double>>{{1,1.5,1},{1,1.5,2},{2,1.5,1},{1.6,1.2,1.5}})
    for(double incidence:{0.0,pi/6})for(const std::string branch:{"RMAX","RMIN","TMAX","TMIN"})
      for(double m:{0.0,1.0,3.0}){
        const double n0=indices[0],nf=indices[1],ns=indices[2],wavelength=600e-9;
        const double beta=std::asin(n0*std::sin(incidence)/nf),cosine=std::cos(beta);
        const double front=nf>n0?pi:0,back=ns>nf?pi:0;
        const bool reflectionMax=branch=="RMAX"||branch=="TMIN";
        const double desired=reflectionMax?1:-1;
        const std::string prefix="O_FILM_"+branch;
        const auto r=solve(prefix+"_D",{n0,nf,ns,incidence,wavelength,m});
        const double d=value(r,"d","m");
        equal(value(r,"beta Film","rad"),beta,"Snell in film",2e-11,1e-15);
        equal(value(r,"Phase vorn","rad"),front,"front reflection phase");
        equal(value(r,"Phase hinten","rad"),back,"back reflection phase");
        equal(std::cos(4*pi*nf*d*cosine/wavelength+back-front),desired,"independent film phase condition");
        equal(value(r,"Gangunterschied","m"),2*nf*d*cosine,"film optical path");
        need(r.note.find("keine absolute Intensitaet")!=std::string::npos,"no absolute film intensity");
        if(d==0)reject(prefix+"_LAMBDA",{n0,nf,ns,incidence,d,m});
        else expect(prefix+"_LAMBDA",{n0,nf,ns,incidence,d,m},"lambda",wavelength,"m");
      }
  expect("O_FILM_RMAX_D",{1,1.5,1,0,600e-9,0},"d",100e-9,"m");
  expect("O_FILM_TMAX_D",{1,1.5,2,0,600e-9,0},"d",100e-9,"m");
  expect("O_FILM_RMIN_D",{1,1.5,1,0,600e-9,0},"d",0,"m");
  for(const auto& [id,v]:std::vector<std::pair<std::string,std::vector<double>>>{
      {"O_FILM_RMAX_D",{1,1,1.5,0,600e-9,1}},
      {"O_FILM_RMAX_D",{1,1.5,1.5,0,600e-9,1}},
      {"O_FILM_RMAX_D",{1.5,1,1.4,pi/3,600e-9,1}},
      {"O_FILM_RMAX_D",{1.5,1.7,1,pi/3,600e-9,1}},
      {"O_FILM_RMAX_D",{1,1.5,1,pi/2,600e-9,1}},
      {"O_FILM_RMAX_D",{1,1.5,1,0,600e-9,1.5}},
      {"O_FILM_RMAX_D",{1,1.5,1,0,0,1}},
      {"O_FILM_RMIN_LAMBDA",{1,1.5,1,0,100e-9,0}},
      {"O_FILM_RMAX_LAMBDA",{1,1.5,2,0,100e-9,0}},
      {"O_FILM_TMIN_LAMBDA",{1,1.5,2,0,100e-9,0}}})reject(id,v);
}
void slitAndResolution(){
  const double lambda=600e-9,b=1e-6,a=std::atan2(3.0,4.0);
  expect("O_SS_ANGLE",{lambda,b,1},"theta",a,"rad");
  expect("O_SS_LAMBDA",{b,1,a},"lambda",lambda,"m");
  expect("O_SS_B",{lambda,1,a},"b",b,"m");
  expect("O_SS_ORDER",{lambda,b,a},"k",1);
  expect("O_SS_SCREEN",{lambda,b,1,4},"x einseitig",3,"m");
  expect("O_SS_LAMBDA_SCREEN",{b,1,4,3},"lambda",lambda,"m");
  expect("O_SS_B_SCREEN",{lambda,1,4,3},"b",b,"m");
  expect("O_SS_ORDER_SCREEN",{lambda,b,4,3},"k",1);
  expect("O_SS_L_SCREEN",{lambda,b,1,3},"L",4,"m");
  expect("O_SS_CENTRAL",{lambda,b,4},"W",6,"m");
  expect("O_SS_CENTRAL_B",{lambda,4,6},"b",b,"m");
  expect("O_SS_CENTRAL_LAMBDA",{b,4,6},"lambda",lambda,"m");
  expect("O_SS_CENTRAL_L",{lambda,b,6},"L",4,"m");
  reject("O_SS_ANGLE",{lambda,b,0});reject("O_SS_ORDER",{lambda,b,0});
  reject("O_SS_ORDER_SCREEN",{lambda,b,4,0});reject("O_SS_ANGLE",{lambda,b,1.2});
  reject("O_SS_SCREEN",{lambda,lambda,1,4});reject("O_SS_CENTRAL",{lambda,lambda,4});
  reject("O_SS_CENTRAL",{lambda,lambda/2,4});reject("O_SS_CENTRAL_L",{lambda,b,0});
  expect("O_GRATING_R",{2,1000},"R",2000);
  expect("O_GRATING_N",{1001,2},"N minimal",501);
  expect("O_GRATING_N",{1001,2},"R",1002);
  expect("O_GRATING_K",{1001,500},"k minimal",3);
  expect("O_GRATING_RESOLUTION",{600e-9,2,1000},"Delta lambda",.3e-9,"m");
  expect("O_RAYLEIGH",{500e-9,.01},"theta Rayleigh",std::asin(6.1e-5),"rad");
  expect("O_RAYLEIGH_D",{500e-9,std::asin(6.1e-5)},"D",.01,"m");
  expect("O_RAYLEIGH_LAMBDA",{.01,std::asin(6.1e-5)},"lambda",500e-9,"m");
  reject("O_GRATING_R",{0,1000});reject("O_GRATING_R",{2,1000.5});
  reject("O_GRATING_N",{0,2});reject("O_GRATING_K",{1001,0});
  reject("O_RAYLEIGH",{1,1});reject("O_RAYLEIGH_D",{500e-9,0});
  reject("O_RAYLEIGH_LAMBDA",{.01,pi});
}
void originalAndQuantum(){
  // Original Aufgabenheft p.21: 600 lines/mm, L=1.50m, x=56.7cm (one side!).
  const double d=1/(600*1000.0),L=1.5;
  const auto na=solve("O_DG_LAMBDA_SCREEN",{d,1,L,input("56,7",.01)});
  equal(value(na,"lambda","m"),589.304e-9,"Abiheft 17a",2e-6);
  const auto doublet=solve("O_DG_SEPARATION",{input("589,0",1e-9),input("589,6",1e-9),d,2,L});
  equal(value(doublet,"Linienabstand","m"),3.055e-3,"Abiheft 17e",4e-4);
  // Unit changes are explicit parser inputs, then the public API takes SI only.
  expect("O_DG_LAMBDA_SCREEN",{input("1,6666666666666667",1e-6),1,input("1500",1e-3),input("567",1e-3)},
         "lambda",value(na,"lambda","m"),"m");
  expect("O_SS_CENTRAL",{input("600",1e-9),input("1",1e-6),input("400",.01)},"W",6,"m");
  expect("O_SNELL_BETA",{1,1.5,input("30",degrees)},"beta",std::asin(1.0/3),"rad");
  const auto electron=solve("O_ELECTRON_COMPARE",{input("50",1000)});
  equal(value(electron,"lambda klassisch","m"),5.48474e-12,"Abiheft 9g classical",2e-6);
  equal(value(electron,"lambda relativistisch","m"),5.35531e-12,"Abiheft 9g relativistic",2e-6);
  equal(value(electron,"Relativer Fehler","%"),2.41698,"Abiheft 9g relative error",2e-6);
  equal(value(electron,"Klassisch unter 0.1 c",""),0,"50kV flagged beyond classical limit");
  need(electron.note.find("WARNUNG")!=std::string::npos,"explicit high-voltage model warning");
  equal(value(solve("O_ELECTRON_COMPARE",{1000}),"Klassisch unter 0.1 c",""),1,"1kV within stated approximation");
  // Original embedded equations on p.19: d=201pm, U=30kV, theta_min=5.9deg.
  expect("O_THRESHOLD_H",{input("30",1000),input("201",1e-12),input("5,9",degrees),1},
         "h",6.62519e-34,"J s",2e-6);
  const auto cu=solve("O_MOSELEY_Z",{154e-12,1});
  equal(value(cu,"Z geschaetzt",""),29.095,"Abiheft 15e rounded to three decimals",2e-5);
  equal(value(cu,"Z naechste ganze Zahl",""),29,"Cu Z");need(cu.note.find("Cu (Kupfer)")!=std::string::npos,"Cu identification");
  // Independently generate wavelength from a prescribed K-alpha energy, with non-default shielding.
  for(double Z:{6.0,29.0,80.0})for(double shield:{0.0,1.0,2.5}){
    const double energy=10.2*charge*(Z-shield)*(Z-shield),lambda=planck*light/energy;
    expect("O_MOSELEY_Z",{lambda,shield},"Z geschaetzt",Z);
    expect("O_MOSELEY_LAMBDA",{Z,shield},"lambda K-alpha",lambda,"m");
  }
  const double spacing=200e-12;
  // Independent synthetic threshold with third-order Bragg reflection.
  const double volts=50000,order=3,angle=std::asin(order*planck*light/(2*spacing*charge*volts));
  expect("O_THRESHOLD_H",{volts,spacing,angle,order},"h",planck,"J s");
  for(double voltage:{1.0,1000.0,50000.0,1e7}){
    // p_rel^2 c^2 = (eU)^2 + 2 eU mc^2, a physically independent formula.
    const double kinetic=charge*voltage,momentum=std::sqrt(kinetic*kinetic+2*kinetic*electronMass*light*light)/light;
    const auto r=solve("O_ELECTRON_COMPARE",{voltage});
    equal(value(r,"lambda relativistisch","m"),planck/momentum,"relativistic energy-momentum relation");
    const double classical=value(r,"lambda klassisch","m"),rel=value(r,"lambda relativistisch","m");
    equal(value(r,"Relativer Fehler","%"),100*(classical-rel)/rel,"relative error uses relativistic denominator",2e-9);
    const double gamma=1+kinetic/(electronMass*light*light);
    equal(value(r,"v klassisch","m/s"),std::sqrt(2*kinetic/electronMass),"classical speed as comparison");
    equal(value(r,"v relativistisch","m/s"),momentum/(gamma*electronMass),"relativistic speed from p=gamma*m*v");
  }
  for(const auto& [id,v]:std::vector<std::pair<std::string,std::vector<double>>>{
      {"O_ELECTRON_COMPARE",{0}},{"O_ELECTRON_COMPARE",{-50000}},
      {"O_THRESHOLD_H",{30000,201e-12,0,1}},{"O_THRESHOLD_H",{30000,201e-12,pi,1}},
      {"O_THRESHOLD_H",{30000,201e-12,.1,1.5}},
      {"O_MOSELEY_Z",{154e-12,-1}},{"O_MOSELEY_Z",{1e-20,1}},
      {"O_MOSELEY_LAMBDA",{29.1,1}},{"O_MOSELEY_LAMBDA",{29,29}},
      {"O_MOSELEY_LAMBDA",{119,1}}})reject(id,v);
}
void oldRegressions(){
  for(const std::string id:{"Q_DS","Q_DL","Q_DD","Q_DDIST"}){
    Calculation c{"old","old","x","m",id,{{"a","",""},{"b","",""},{"c","",""}}};
    double answer=0;std::string error;
    need(calculate(c,{500e-9,2,.5e-3},answer,error),id+" unchanged approximate formula");
    equal(answer,.002,id+" old paraxial result");
  }
  Calculation classical{"old","old","lambda","m","Q_LUC",{{"U","V","V"}}};
  double result=0;std::string error;
  need(!calculate(classical,{50000},result,error)&&error.find("0.1 c")!=std::string::npos,"old classical 0.1c guard retained");
  need(calculate(classical,{1000},result,error),"old classical small-U still works");
  equal(result,value(solve("O_ELECTRON_COMPARE",{1000}),"lambda klassisch","m"),"comparison matches valid old classical path");
}
}
int main(){
  catalogue();apiValidation();diffraction();standingWaves();refraction();polarization();
  thinFilms();slitAndResolution();originalAndQuantum();oldRegressions();
  std::cout<<"PASS OPTICS: "<<checks<<" checks; 8 categories; 71 workflows (69 fixed, 2 dynamic); original 17a/17e, 9g, 15d/15e; inverse, SI/domain, old-formula regressions\n";
}

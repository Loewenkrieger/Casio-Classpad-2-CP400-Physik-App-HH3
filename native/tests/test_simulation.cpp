// SPDX-License-Identifier: GPL-3.0-or-later
#include "extensions.hpp"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <set>

namespace {
using namespace physik;
unsigned checks=0;
void need(bool condition,const std::string& message){
  ++checks;
  if(!condition){std::cerr<<"FAIL SIMULATION: "<<message<<'\n';std::exit(1);}
}
void near(double actual,double expected,const std::string& message,double rel=1e-7,double abs=1e-10){
  if(!(std::isfinite(actual)&&std::isfinite(expected)&&
       std::abs(actual-expected)<=std::max(abs,rel*std::abs(expected)))){
    std::cerr<<message<<": actual="<<actual<<", expected="<<expected<<'\n';
    need(false,message);
  }
  ++checks;
}
double get(const NativeResult& r,const std::string& name){
  for(const auto& x:r.values)if(x.name==name)return x.value;
  need(false,"missing value: "+name);return 0;
}
bool has(const NativeResult& r,const std::string& name){
  for(const auto& x:r.values)if(x.name==name)return true;
  return false;
}
void healthy(const NativeResult& r){
  need(!r.values.empty()&&!r.note.empty(),"named/text results");
  need(get(r,"RK4-Schritte gesamt")<=50000,"global step budget including check/search/events");
  std::set<std::string> names;
  for(const auto& x:r.values){need(std::isfinite(x.value),"finite result");need(names.insert(x.name).second,"unique result name");}
  need(r.plots.size()<=12,"plot memory bound");
  for(const auto& p:r.plots){
    need(!p.title.empty()&&!p.xLabel.empty()&&!p.yLabel.empty(),"defined axes");
    need(!p.points.empty()&&p.points.size()<=400&&p.model.size()<=400,"bounded stored points");
    for(const auto* series:{&p.points,&p.model})for(auto xy:*series)
      need(std::isfinite(xy.first)&&std::isfinite(xy.second),"finite plot point");
    for(size_t i=1;i<p.points.size();++i)need(p.points[i].first>=p.points[i-1].first,"nonreversing x axis");
  }
}
NativeResult solve(const std::string& id,const std::vector<double>& v){
  NativeResult r;std::string error="stale error";
  const bool ok=simulationCalculate(id,v,r,error);
  need(ok,id+": "+error);need(error.empty(),"clear error on success");healthy(r);return r;
}
void reject(const std::string& id,const std::vector<double>& v,const std::string& context,const std::string& contains=""){
  NativeResult r;r.values.push_back({"stale",1,"m"});r.note="stale";
  r.plots.push_back({"stale","x","m","y","m",{{0,0}},{},true});
  std::string error;
  need(!simulationCalculate(id,v,r,error),context+": reject");
  need(!error.empty(),context+": German error");
  need(r.values.empty()&&r.note.empty()&&r.plots.empty(),context+": clear all stale/partial results");
  if(!contains.empty())need(error.find(contains)!=std::string::npos,context+": expected error text: "+error);
}
std::vector<double> defaults(const Workflow& w){
  std::vector<double> v;
  for(const auto& text:w.defaults){char* end=nullptr;v.push_back(std::strtod(text.c_str(),&end));need(end&&!*end,"numeric default");}
  if(w.maxCount)for(unsigned i=0;i<static_cast<unsigned>(v[0]);++i)
    for(const auto& text:w.repeatedDefaults)v.push_back(std::strtod(text.c_str(),nullptr));
  return v;
}
const std::vector<double> rl={15,7,1.5,0,1,.001};
const std::vector<double> cart={.05,25,.04,.02,0,0,5,.001};
const std::vector<double> fall={.0018,.0178,1.29,.6,2,0,0,9.81,3,.001,1};

// Oracles use closed forms, not production integration or result-to-result round trips.
double current(double u,double r,double l,double i0,double t){return u/r+(i0-u/r)*std::exp(-r*t/l);}
double dragK(double cw=.6){return .5*1.29*cw*.0178/.0018;}
double fallingTime(double distance,double cw=.6){
  if(cw==0)return std::sqrt(2*distance/9.81);
  const double k=dragK(cw);return std::acosh(std::exp(k*distance))/std::sqrt(9.81*k);
}
double fallVelocitySquared(double s,double cw=.6){
  const double k=dragK(cw);return (9.81/k)*(-std::expm1(-2*k*s));
}
void catalogue(){
  const auto categories=simulationCategories();need(categories.size()==3,"exact category count");
  std::set<std::string> ids,cats;unsigned count=0,dynamic=0;
  for(const auto& c:categories){
    need(!c.title.empty()&&cats.insert(c.id).second,"unique nonempty category");
    const auto entries=simulationCatalog(c.id);need(!entries.empty(),"nonempty category");
    for(const auto& w:entries){
      ++count;const auto& calc=w.calculation;
      need(calc.id.rfind("S_",0)==0&&ids.insert(calc.id).second,"stable unique S_ ID");
      need(!calc.title.empty()&&!calc.formula.empty(),"title and equation");
      need(calc.parameters.size()==w.defaults.size(),"base defaults match parameters");
      need(w.minCount<=w.maxCount&&w.maxCount<=40,"repeat bounds");
      if(w.maxCount){++dynamic;need(w.repeated.size()==w.repeatedDefaults.size(),"repeated defaults");}
      for(const auto& p:calc.parameters){need(!p.name.empty(),"parameter name");if(p.unit=="rad")need(p.defaultUnit=="deg","degree UI contract");}
      const auto v=defaults(w);solve(calc.id,v);
      auto bad=v;bad.pop_back();reject(calc.id,bad,"missing input");
      bad=v;bad.push_back(1);reject(calc.id,bad,"extra input");
      for(size_t i=0;i<v.size();++i){
        bad=v;bad[i]=std::numeric_limits<double>::quiet_NaN();reject(calc.id,bad,"NaN input");
        bad[i]=std::numeric_limits<double>::infinity();reject(calc.id,bad,"infinite input");
        bad[i]=-std::numeric_limits<double>::infinity();reject(calc.id,bad,"negative infinite input");
      }
    }
  }
  need(count==7&&dynamic==1,"exact workflow counts: 7 (1 dynamic)");
  need(simulationCatalog("S_unknown").empty(),"unknown category");
  for(const auto& id:{"S_","S_RL_extra","S_FALL_extra","M_RL",""})reject(id,rl,"exact known ID validation");
}
void originalRL(){
  const auto a=solve("S_RL",rl);
  near(get(a,"I(t Ende)"),current(15,7,1.5,0,1),"2013 I(1)",2e-10);
  near(get(a,"tau"),1.5/7,"2013 tau");near(get(a,"I unendlich"),15.0/7,"2013 steady current");
  near(get(a,"t99 Restfehler"),std::log(100.0)*1.5/7,"2013 99% criterion");
  need(a.plots.size()==1&&a.plots[0].xUnit=="s"&&a.plots[0].yUnit=="A","I(t) axis units");
  for(const auto& xy:a.plots[0].points)near(xy.second,current(15,7,1.5,0,xy.first),"analytic RL trace",3e-10);
  auto v=rl;v[3]=4;const auto decay=solve("S_RL",v);
  near(get(decay,"I(t Ende)"),current(15,7,1.5,4,1),"adjusted initial current");
  v[3]=15.0/7;near(get(solve("S_RL",v),"t99 Restfehler"),0,"already stationary");
  v=rl;v[0]=-15;v[3]=-1;near(get(solve("S_RL",v),"I(t Ende)"),current(-15,7,1.5,-1,1),"signed current/source");
  const double measured=current(12.2,15,2.9,0,1);
  const auto inverse=solve("S_RL_R",{12.2,2.9,0,1,measured,1,40,1,.001});
  near(get(inverse,"R aus Messwert"),15,"2013 inverse R",1e-9);
  near(get(inverse,"I am Messzeitpunkt"),measured,"2013 inverse RK4 model check");
  std::cout<<"Original RL: I(1)="<<get(a,"I(t Ende)")<<" A, R inverse="<<get(inverse,"R aus Messwert")<<" Ohm\n";
}
void originalSpring(){
  const auto a=solve("S_SPRING",cart);
  const double omega=std::sqrt(25/.05),eq=.04-.02/25;
  const double releaseTime=std::acos((.04-eq)/(-eq))/omega;
  const double releaseSpeed=std::sqrt((25*.04*.04-2*.02*.04)/.05);
  near(get(a,"s Ende"),1,"2014 exact stop 1.000 m, not Euler artifact 1.01 m",2e-9);
  near(get(a,"t Feder frei"),releaseTime,"spring release localized",2e-8);
  near(get(a,"v Feder frei"),releaseSpeed,"spring release speed",2e-8);
  near(get(a,"t Ende"),releaseTime+releaseSpeed*.05/.02,"first post-start rest time",2e-8);
  near(get(a,"v maximal"),eq*omega,"true peak velocity",2e-8);
  near(get(a,"v Ende"),0,"no fake reversal");near(get(a,"Stillstand erreicht"),1,"stop flag");
  near(get(a,"Energierest"),0,"spring energy balance",0,1e-10);
  auto longRun=cart;longRun[6]=100;
  near(get(solve("S_SPRING",longRun),"s Ende"),1,"long horizon stops early within actual budget");
  for(const auto& xy:a.plots[0].points){
    need(xy.second>=0&&xy.first<=1+1e-8,"nonreversal/bounded distance");
    const double c=std::max(0.0,.04-xy.first);
    near(.5*.05*xy.second*xy.second+.5*25*c*c+.02*xy.first,.02,"energy at each sample",3e-8);
  }
  auto v=cart;v[3]=.75;const auto inside=solve("S_SPRING",v);
  near(get(inside,"s Ende"),.02,"first rest before spring release",3e-8);
  near(get(inside,"t Ende"),pi/omega,"rest inside spring localized",3e-8);
  need(!has(inside,"t Feder frei"),"no fictitious spring release");
  for(double force:{1.0,1.1}){
    v=cart;v[3]=force;const auto still=solve("S_SPRING",v);
    near(get(still,"s Ende"),0,"initial force <= friction, no motion");
    near(get(still,"t Ende"),0,"no negative first step");near(get(still,"RK4-Schritte gesamt"),0,"static state costs no integration");
  }
  v=cart;v[3]=0;const auto free=solve("S_SPRING",v);
  near(get(free,"Stillstand erreicht"),0,"frictionless no fabricated stop");
  near(get(free,"v Ende"),std::sqrt(25/.05)*.04,"frictionless released velocity",2e-8);
  v=cart;v[4]=.04;v[5]=.2;
  const auto initial=solve("S_SPRING",v);
  near(get(initial,"s Ende"),.04+.5*.05*.2*.2/.02,"adjusted initial s and v");
  v=cart;v[6]=.05;const auto truncated=solve("S_SPRING",v);
  need(get(truncated,"Stillstand erreicht")==0&&truncated.note.find("Zeithorizont")!=std::string::npos,"honest finite horizon");
  const auto inverse=solve("S_SPRING_FR",{.05,25,.04,0,0,.59,.005,.5,5,.001});
  near(get(inverse,"FR aus Zielstillstand"),.02/.59,"2014 target stop FR",1e-9);
  near(get(inverse,"s Ende"),.59,"2014 inverse RK4 stop",2e-8);
  const auto comparison=solve("S_SPRING_COMPARE",{2,.05,25,.04,0,0,5,.001,.02,.034});
  need(comparison.plots.size()==4,"comparison has both axes per run");
  near(get(comparison,"s Ende #1"),1,"comparison FR1");near(get(comparison,"s Ende #2"),.02/.034,"comparison FR2");
  std::cout<<"Original spring: stop="<<get(a,"s Ende")<<" m, FR(target 0.59)="<<get(inverse,"FR aus Zielstillstand")<<" N\n";
}
void originalFall(){
  const auto a=solve("S_FALL",fall);const double k=dragK(),vt=std::sqrt(9.81/k);
  near(get(a,"Fallzeit"),fallingTime(2),"2016 localized fall time",2e-9);
  near(get(a,"s Ende"),2,"no ground overshoot");near(get(a,"v Ende"),std::sqrt(fallVelocitySquared(2)),"ground velocity");
  near(get(a,"Grenzgeschwindigkeit"),vt,"terminal speed");
  const double work=.0018*(9.81-.5*fallVelocitySquared(1));
  near(get(a,"WR am Messweg"),work,"2016 positive WR at s=1",1e-9);
  near(get(a,"WR / mgh am Messweg"),100*work/(.0018*9.81*2),"2016 43.47%, not rounded 42%",1e-9);
  near(get(a,"t99 Geschwindigkeit"),std::atanh(.99)/std::sqrt(9.81*k),"99% time",2e-9);
  near(get(a,"s99 Geschwindigkeit"),-std::log(1-.99*.99)/(2*k),"99% distance",2e-9);
  near(get(a,"Energierest"),0,"fall energy balance",0,1e-11);
  auto longRun=fall;longRun[8]=100;
  near(get(solve("S_FALL",longRun),"Fallzeit"),fallingTime(2),"long fall horizon stops early within budget");
  need(a.plots.size()==3,"all required plots");
  need(a.plots[0].xUnit=="m"&&a.plots[0].yUnit=="N"&&a.plots[1].yUnit=="J"&&a.plots[2].xUnit=="s","fall axis units");
  for(const auto& xy:a.plots[0].points)near(xy.second,.0018*9.81*std::exp(-2*k*xy.first),"analytic net force",1e-5,1e-11);
  for(const auto& xy:a.plots[1].points){
    need(xy.second>=0,"positive work");
    near(xy.second,.0018*(9.81*xy.first-.5*fallVelocitySquared(xy.first)),"analytic work trace",2e-8);
  }
  auto v=fall;v[3]=0;const auto vacuum=solve("S_FALL",v);
  near(get(vacuum,"Fallzeit"),std::sqrt(4/9.81),"vacuum analytic fall",2e-10);
  near(get(vacuum,"WR Ende"),0,"vacuum zero work");need(!has(vacuum,"Grenzgeschwindigkeit"),"no infinite terminal result");
  v=fall;v[5]=.2;v[6]=.4;const auto initial=solve("S_FALL",v);
  const double phase=std::atanh(.4/vt),target=std::acosh(std::exp(k*(2-.2))*std::cosh(phase));
  near(get(initial,"Fallzeit"),(target-phase)/std::sqrt(9.81*k),"adjusted fall s0,v0",3e-9);
  v=fall;v[6]=2*vt;const auto fast=solve("S_FALL",v);
  need(get(fast,"WR Ende")>0&&get(fast,"v Ende")>vt,"initial speed above terminal is physical");
  v=fall;v[8]=.1;const auto shortRun=solve("S_FALL",v);
  need(get(shortRun,"Boden erreicht")==0&&!has(shortRun,"Fallzeit")&&!has(shortRun,"WR am Messweg"),"no invented horizon events");
  const auto inverse=solve("S_FALL_CW",{.0018,.0178,1.29,2,0,0,9.81,fallingTime(2,.8),.1,2,3,.001,1});
  near(get(inverse,"cW aus Fallzeit"),.8,"inverse fall independently generated cW",6e-6);
  near(get(inverse,"Fallzeit"),fallingTime(2,.8),"inverse fall model check",6e-6);
  const auto original=solve("S_FALL_CW",{.0018,.0178,1.29,2,0,0,9.81,1.85,.1,2,3,.001,1});
  near(fallingTime(2,get(original,"cW aus Fallzeit")),1.85,"original measured 1.85 s analytic model check",6e-6);
  need(get(original,"cW aus Fallzeit")>1.15&&get(original,"cW aus Fallzeit")<1.25,"2016 cW near 1.2");
  std::cout<<"Original fall: time="<<get(a,"Fallzeit")<<" s, WR/mgh="<<get(a,"WR / mgh am Messweg")
           <<"%, cW(1.85s)="<<get(original,"cW aus Fallzeit")<<"\n";
}
void scalesAndConvergence(){
  auto v=rl;v[0]*=1000;v[1]*=1000;v[2]*=1000;
  near(get(solve("S_RL",v),"I(t Ende)"),current(15,7,1.5,0,1),"RL parameter-scale invariance");
  v={50*.001,25,4*.01,20*.001,0,0,5,1*.001};
  near(get(solve("S_SPRING",v),"s Ende"),1,"SI inputs from g/cm/mN/ms");
  v=cart;v[0]*=100;v[1]*=100;v[3]*=100;
  near(get(solve("S_SPRING",v),"s Ende"),1,"mass/spring/friction scale invariance");
  v={1.8*.001,178*1e-4,1.29,.6,200*.01,0,0,9.81,3,1*.001,100*.01};
  near(get(solve("S_FALL",v),"Fallzeit"),fallingTime(2),"SI inputs from g/cm^2/cm/ms");
  v=fall;v[0]*=10;v[1]*=10;
  near(get(solve("S_FALL",v),"Fallzeit"),fallingTime(2),"fall mass/area scale invariance");
  v=rl;v[5]=.4;const auto coarse=solve("S_RL",v);v[5]=.2;const auto half=solve("S_RL",v);
  need(std::abs(get(half,"I(t Ende)")-current(15,7,1.5,0,1))<std::abs(get(coarse,"I(t Ende)")-current(15,7,1.5,0,1)),"RL convergence");
  need(coarse.note.find("WARNUNG")!=std::string::npos&&get(coarse,"Abweichung dt/2 maximal")>.1,"warn >0.1%, never silent");
  v=cart;v[7]=.04;const auto sc=solve("S_SPRING",v);v[7]=.02;const auto sh=solve("S_SPRING",v);
  need(std::abs(get(sh,"s Ende")-1)<std::abs(get(sc,"s Ende")-1),"event-safe spring convergence");
  v=fall;v[9]=.12;const auto fc=solve("S_FALL",v);v[9]=.06;const auto fh=solve("S_FALL",v);
  need(std::abs(get(fh,"Fallzeit")-fallingTime(2))<std::abs(get(fc,"Fallzeit")-fallingTime(2)),"ground event convergence");
  need(fc.note.find("WARNUNG")!=std::string::npos,"coarse fall warns");
}
void invalid(){
  for(double dt:{0.0,-.001,2.0,1e-300,std::numeric_limits<double>::denorm_min()}){
    auto v=rl;v[5]=dt;reject("S_RL",v,"invalid RK4 step");
  }
  for(unsigned i:{1u,2u}){auto v=rl;v[i]=0;reject("S_RL",v,"nonpositive RL physical domain");}
  auto v=rl;v[1]=1e6;reject("S_RL",v,"unstable RL step");
  v=rl;v[0]=std::numeric_limits<double>::max();v[2]=.001;reject("S_RL",v,"finite inputs but overflowing model");
  v=rl;v[1]=1e-300;v[2]=1e300;reject("S_RL",v,"underflowing positive RL rate");
  for(unsigned i:{0u,1u,2u,3u,4u,5u,6u,7u}){
    v=cart;v[i]=-1;reject("S_SPRING",v,"negative cart domain");
  }
  v=cart;v[0]=0;reject("S_SPRING",v,"zero mass");v=cart;v[1]=0;reject("S_SPRING",v,"zero spring");
  v=cart;v[7]=.1;reject("S_SPRING",v,"unstable spring step");
  v=cart;v[0]=1e300;v[1]=1e-300;reject("S_SPRING",v,"underflowing positive spring rate");
  v=cart;v[0]=1e300;v[1]=1e300;v[3]=1e-300;reject("S_SPRING",v,"underflowing positive friction rate");
  for(unsigned i=0;i<fall.size();++i){v=fall;v[i]=-1;reject("S_FALL",v,"negative fall domain");}
  v=fall;v[0]=0;reject("S_FALL",v,"zero falling mass");v=fall;v[5]=2;reject("S_FALL",v,"start at/below ground");
  v=fall;v[10]=2.1;reject("S_FALL",v,"probe below ground");v=fall;v[9]=.2;reject("S_FALL",v,"unstable fall step");
  v=fall;v[0]=1e300;v[1]=1e-300;reject("S_FALL",v,"underflowing positive drag rate");
  for(double count:{0.0,-1.0,1.5,5.0,1e30})reject("S_SPRING_COMPARE",{count},"invalid repeat count");
  reject("S_SPRING_COMPARE",{2,.05,25,.04,0,0,5,.001,.02,-.1},"late failure clears previous comparison");
  reject("S_SPRING_COMPARE",{4,.05,25,.04,0,0,16,.001,.0001,.0001,.0001,.0001},"shared total integration budget","50000");
  reject("S_RL_R",{15,1.5,0,.5,10,.1,50,1,.001},"R target outside bracket");
  reject("S_RL_R",{15,1.5,0,.5,1,10,1,1,.001},"reversed R bracket");
  reject("S_RL_R",{15,1.5,0,.5,1,0,50,1,.001},"zero R bracket");
  reject("S_RL_R",{15,1.5,0,2,1,.1,50,1,.001},"measurement after horizon");
  reject("S_SPRING_FR",{.05,25,.04,0,0,100,.005,.5,5,.001},"stop not bracketed");
  reject("S_SPRING_FR",{.05,25,.04,0,0,.59,.5,.005,5,.001},"reversed friction bracket");
  reject("S_SPRING_FR",{.05,25,.04,0,0,.59,0,.5,5,.001},"zero friction bracket");
  reject("S_SPRING_FR",{.05,25,.04,0,0,.59,.005,.5,.1,.001},"target beyond time horizon");
  reject("S_FALL_CW",{.0018,.0178,1.29,2,0,0,9.81,.1,.1,2,3,.001,1},"fall time outside bracket");
  reject("S_FALL_CW",{.0018,.0178,1.29,2,0,0,9.81,1.85,2,.1,3,.001,1},"reversed drag bracket");
  reject("S_FALL_CW",{.0018,0,1.29,2,0,0,9.81,1.85,.1,2,3,.001,1},"unidentifiable drag");
  reject("S_FALL_CW",{.0018,.0178,1.29,2,0,0,9.81,1.85,.1,20,2,.001,1},"bracket fails finite-horizon model check");
}
}
int main(){
  catalogue();originalRL();originalSpring();originalFall();scalesAndConvergence();invalid();
  std::cout<<"PASS SIMULATION: "<<checks<<" checks; 3 categories, 7 workflows (1 dynamic); RK4/original/inverse/SI/domain/events/convergence/budget\n";
}

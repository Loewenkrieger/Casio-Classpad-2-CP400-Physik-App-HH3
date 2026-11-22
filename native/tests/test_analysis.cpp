// SPDX-License-Identifier: GPL-3.0-or-later
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
using Points=std::vector<std::pair<double,double>>;
int checks=0;
std::map<std::string,std::vector<double>> examples;
void need(bool value,const std::string& message){
  ++checks;if(!value){std::cerr<<"FAIL ANALYSIS: "<<message<<'\n';std::exit(1);}
}
void near(double actual,double expected,const std::string& message,double relative=2e-10,double absolute=0){
  need(std::isfinite(actual)&&std::abs(actual-expected)<=absolute+relative*std::abs(expected),
    message+": expected "+formatNumber(expected,12)+", got "+formatNumber(actual,12));
}
bool has(const NativeResult& r,const std::string& key){
  for(const auto& value:r.values)if(value.name==key)return true;
  return false;
}
double get(const NativeResult& r,const std::string& key){
  for(const auto& value:r.values)if(value.name==key)return value.value;
  need(false,"missing value "+key);return 0;
}
void bounded(const NativeResult& r){
  need(!r.values.empty(),"nonempty results");
  for(const auto& value:r.values)need(!value.name.empty()&&!value.unit.empty()&&std::isfinite(value.value),"named finite value");
  for(const auto& p:r.plots){
    need(!p.title.empty()&&!p.xLabel.empty()&&!p.yLabel.empty()&&!p.xUnit.empty()&&!p.yUnit.empty(),"labelled axes");
    need(p.points.size()<=400&&p.model.size()<=400,"bounded plots");
    for(const auto* list:{&p.points,&p.model})for(auto [x,y]:*list)need(std::isfinite(x)&&std::isfinite(y),"finite plot");
  }
}
NativeResult run(const std::string& id,const std::vector<double>& v){
  NativeResult r;r.note="stale";std::string error="stale";
  need(analysisCalculate(id,v,r,error),id+": "+error);need(error.empty(),"clear success error");bounded(r);
  examples[id]=v;return r;
}
void reject(const std::string& id,const std::vector<double>& v){
  NativeResult r;r.note="stale";r.values={{"stale",1,"1"}};r.plots.push_back({});std::string error;
  need(!analysisCalculate(id,v,r,error),"reject "+id);need(!error.empty(),"German failure diagnostic");
  need(r.values.empty()&&r.plots.empty()&&r.note.empty(),"failed call clears all stale results");
}
double si(const std::string& number,double scale){double x=0;std::string error;need(parseNumber(number,scale,x,error),"unit parsing");return x;}
void catalogs(){
  const auto categories=analysisCategories();need(categories.size()==5,"five categories");
  const size_t counts[]={4,10,3,4,5};std::set<std::string> ids,cats;size_t count=0;
  for(size_t i=0;i<categories.size();++i){const auto& c=categories[i];need(cats.insert(c.id).second&&!c.title.empty(),"unique category");
    const auto list=analysisCatalog(c.id);need(list.size()==counts[i],"exact per-category count");
    for(const auto& w:list){++count;const auto& calc=w.calculation;
      need(calc.id.rfind("A_",0)==0&&ids.insert(calc.id).second&&!calc.title.empty(),"unique stable A_ ID");
      need(!calc.formula.empty()&&calc.formula.find('=')!=std::string::npos,"real formula/model shown in form");
      need(w.defaults.size()==calc.parameters.size(),"base defaults match fixed parameters");
      need(w.repeatedDefaults.size()==w.repeated.size(),"repeated defaults match template");
      if(w.maxCount){need(w.minCount==2&&w.maxCount<=40,"bounded manual point counts");
        need(!calc.parameters.empty()&&calc.parameters[0].name=="Punktzahl","count comes first");}
      else need(w.minCount==0&&w.repeated.empty(),"static workflow");
      for(const auto& p:calc.parameters)need(!p.name.empty()&&!p.unit.empty()&&!p.defaultUnit.empty(),"parameter metadata");
      for(const auto& p:w.repeated)need(!p.name.empty()&&!p.unit.empty()&&!p.defaultUnit.empty(),"repeated metadata");
    }
  }
  need(count==26,"26 workflows");need(analysisCatalog("unknown").empty(),"unknown category empty");
}
void diagrams(){
  auto r=run("A_POINTS",{3,.5,2.5,0,2,1,0,3,-2});
  near(get(r,"Flaeche"),-.875,"clipped signed integral");near(get(r,"Flaeche_gesamt"),-1,"full signed integral");
  near(get(r,"Sekante"),-1.25,"selected secant");near(get(r,"m1"),-2,"first secant");near(get(r,"m2"),-1,"second secant");
  need(r.plots.size()==3&&r.plots[1].points.size()==4,"derivative step plot");
  near(r.plots[1].points[1].first,r.plots[1].points[2].first,"vertical step");
  near(r.plots[2].points.back().second,-1,"cumulative full integral");
  need(r.note.find("dimensionslose")!=std::string::npos,"generic points explicitly dimensionless");
  r=run("A_VELOCITY",{3,.5,3.5,0,4,2,0,4,-4});
  near(get(r,"dx"),0,"velocity cancellation",0,1e-14);near(get(r,"a_mittel"),-2,"mean acceleration");
  need(r.plots[1].yUnit=="m/s^2"&&r.plots[2].yUnit=="m","velocity axes");
  r=run("A_FORCE",{3,0,2,0,0,1,10,2,0});near(get(r,"dp"),10,"force triangle momentum");
  need(r.plots[2].yUnit=="N*s","impulse unit");
  r=run("A_FLUX",{3,200,0,2,0,.004,1,.002,2,.001});
  near(get(r,"m1"),.4,"Faraday sign");near(get(r,"m2"),.2,"Faraday second segment");near(get(r,"U_mittel"),.3,"Faraday selected average");
  std::vector<double> many={40,0,39};for(unsigned i=0;i<40;++i){many.push_back(i);many.push_back(2*i+1);}
  r=run("A_POINTS",many);near(get(r,"Flaeche"),1560,"40 point area");
  need(r.plots[1].points.size()==78&&r.plots[2].points.size()==157,"40 point graphs bounded");
  // Restore a small seed for exhaustive malformed-argument tests.
  run("A_POINTS",{3,.5,2.5,0,2,1,0,3,-2});
  reject("A_POINTS",{2,0,1,0,1,0,2});reject("A_POINTS",{2,0,1,1,1,0,2});
  reject("A_POINTS",{2,-1,1,0,1,1,2});reject("A_POINTS",{2,1,1,0,1,1,2});
  reject("A_VELOCITY",{2,-1,1,-1,0,1,1});reject("A_FLUX",{2,1.5,0,1,0,0,1,1});
  reject("A_FLUX",{2,0,0,1,0,0,1,1});
}
void fits(){
  auto r=run("A_LINEAR",{5,0,1.1,1,2.9,2,5.2,3,6.8,4,9});
  near(get(r,"m"),1.97,"linear slope");near(get(r,"b"),1.06,"linear intercept");
  near(get(r,"RMS"),std::sqrt(.091/5),"RMS uses n");near(get(r,"R2"),1-.091/38.9,"R2");
  near(get(r,"sigma_m"),std::sqrt(.091/3/10),"residual covariance slope sigma");
  near(get(r,"sigma_b"),std::sqrt(.091/3*.6),"residual covariance intercept sigma");
  need(r.note.find("systematischen")!=std::string::npos,"statistical not systematic");
  need(r.note.find("dimensionslos")!=std::string::npos,"generic regression explicitly dimensionless");
  r=run("A_LINEAR",{3,0,4,1,4,2,4});need(!has(r,"R2")&&r.note.find("konstantem y nicht definiert")!=std::string::npos,"constant Y R2 undefined");
  for(unsigned n=2;n<=40;++n){std::vector<double> constant={static_cast<double>(n)},constantPower=constant;
    for(unsigned i=0;i<n;++i){constant.insert(constant.end(),{static_cast<double>(i),.7});constantPower.insert(constantPower.end(),{static_cast<double>(i+1),.7});}
    r=run("A_LINEAR",constant);need(!has(r,"R2"),"constant Y undefined for every point count");
    near(get(r,"m"),0,"constant slope exactly zero",0,0);near(get(r,"RMS"),0,"constant RMS exactly zero",0,0);
    r=run("A_POWER",constantPower);need(!has(r,"R2_log")&&!has(r,"R2_original"),"constant power qualities explicitly undefined");
    std::vector<double> duplicateX={static_cast<double>(n)};
    for(unsigned i=0;i<n;++i)duplicateX.insert(duplicateX.end(),{1.,static_cast<double>(i)});
    reject("A_LINEAR",duplicateX);
  }
  r=run("A_LINEAR",{2,0,1,2,5});need(!has(r,"sigma_m")&&r.note.find("2 Punkte")!=std::string::npos,"two points no covariance");
  r=run("A_LINEAR",{3,1e-18,5e-31,2e-18,8e-31,3e-18,11e-31});
  near(get(r,"m"),3e-13,"SI-scale slope");near(get(r,"b"),2e-31,"SI-scale intercept");
  r=run("A_POWER",{3,1,3,2,12,4,48});near(get(r,"A"),3,"power normalization");near(get(r,"p"),2,"power exponent");
  need(has(r,"R2_log")&&has(r,"R2_original")&&has(r,"sigma_p"),"power qualities and errors");
  std::vector<double> exponential={4,5};for(unsigned i=0;i<4;++i){exponential.push_back(i);exponential.push_back(5+2*std::exp(-static_cast<double>(i)));}
  r=run("A_EXP",exponential);near(get(r,"A"),2,"fixed-background amplitude");near(get(r,"k"),-1,"exponential rate");
  r=run("A_DECAY",exponential);near(get(r,"lambda"),1,"decay lambda");near(get(r,"tau"),1,"decay lifetime");near(get(r,"T_half"),std::log(2.0),"half life");
  need(has(r,"sigma_T_half"),"derived decay error");
  std::vector<double> damp={4,0};for(unsigned i=0;i<4;++i){damp.push_back(i*2);damp.push_back(.12*std::exp(-.5*i));}
  r=run("A_DAMPING",damp);near(get(r,"delta"),.25,"amplitude decay rate");near(get(r,"tau"),4,"damping tau");
  // IQB Coulomb, original centimetre/millinewton table, all five measurements.
  std::vector<double> coulombFree={5};const double cm[]={4,6,8,10,15},mN[]={5.92,3.38,1.91,1.23,.55};
  for(unsigned i=0;i<5;++i){coulombFree.push_back(si(formatNumber(cm[i]),.01));coulombFree.push_back(si(formatNumber(mN[i]),.001));}
  r=run("A_COULOMB_POWER",coulombFree);near(get(r,"p"),-1.8198785639048267,"IQB free exponent");
  near(get(r,"A"),1.8444264709644346e-5,"IQB SI coefficient");
  need(std::abs(get(r,"R2_log")-get(r,"R2_original"))>1e-5,"log and original fit quality differ");
  std::vector<double> inverse={4,37e-9};for(unsigned i=1;i<5;++i){inverse.push_back(cm[i]*.01);inverse.push_back(mN[i]*.001);}
  r=run("A_COULOMB",inverse);near(get(r,"m"),1.2120871666014625e-5,"IQB inverse-square slope");
  near(get(r,"b"),1.4603108022537715e-5,"IQB inverse-square offset");near(get(r,"epsilon_0_exp"),8.987931029074465e-12,"IQB epsilon experimental");
  need(r.plots.size()==2&&r.plots[1].xUnit=="1/m^2","inverse-square plot");
  r=run("A_COIL",{4,0,-2e-5,1,6.8e-4,2,1.38e-3,3,2.08e-3});
  near(get(r,"m"),.0007,"B/I");near(get(r,"b"),-2e-5,"coil offset");
  const double ratio=1.75882000838e11,k=.0007;std::vector<double> emData={4,k};
  for(unsigned j=0;j<4;++j){const double I=1+.2*j,radius=.04+.01*j;
    emData.insert(emData.end(),{I,radius,.5*ratio*k*k*I*I*radius*radius+1.5});}
  r=run("A_EM",emData);near(get(r,"e/m"),ratio,"e/m from known B/I");near(get(r,"b"),1.5,"e/m offset");
  const double U=emData[4]-get(r,"b"),I=emData[2];
  near(std::sqrt(2*U/get(r,"e/m"))/(k*I),emData[3],"e/m inverse radius");
  std::vector<double> mo={3};for(double Z:{10.,15.,20.})mo.insert(mo.end(),{Z,1/(1e6*(Z-1)*(Z-1))});
  r=run("A_MOSELEY",mo);near(get(r,"m"),1000,"Moseley slope");near(get(r,"b"),-1000,"Moseley intercept");
  near(get(r,"Abschirmung"),1,"screening");near(get(r,"R_eff"),1e6,"series-dependent R_eff");
  reject("A_LINEAR",{2,1,2,1,3});reject("A_POWER",{2,0,1,2,3});reject("A_POWER",{2,1,-1,2,3});
  reject("A_EXP",{2,5,0,5,1,6});reject("A_DECAY",{2,0,0,1,1,2});reject("A_DAMPING",{2,0,-1,2,1,1});
  reject("A_DECAY",{2,-1,0,1,1,.5});reject("A_DAMPING",{2,-1,0,1,1,.5});
  reject("A_COULOMB",{2,1e-9,0,1,.1,2});reject("A_COULOMB",{2,0,.1,2,.2,1});
  reject("A_COULOMB",{2,1e-9,.1,1,.2,2});reject("A_EM",{2,0,1,.1,10,2,.1,20});
  reject("A_EM",{2,.001,1,-.1,10,2,.1,20});reject("A_MOSELEY",{2,1.5,1e-9,2,1e-10});
  reject("A_MOSELEY",{2,1,0,2,1e-10});
}
void millikanBatch(){
  // IQB Material 3.1, all 32 voltages, not a reduced subset.
  const double U[]={598,197,119,197,592,197,119,197,119,197,119,197,115,197,119,195,
    589,197,590,199,589,197,119,197,118,293,590,148,589,147,590,299};
  std::vector<double> radii={32,.001,9.81,0,0,2300,0};
  const double mass=9.634217471008698e-15;
  std::vector<double> masses={32,.001,9.81,0,0},qs={32};
  for(double u:U){radii.insert(radii.end(),{si("1",1e-6),u,0,0});masses.insert(masses.end(),{mass,u,0,0});qs.insert(qs.end(),{mass*9.81*.001/u,0});}
  const auto rr=run("A_MILLIKAN_R",radii),rm=run("A_MILLIKAN_M",masses),rq=run("A_MILLIKAN_Q",qs);
  near(get(rr,"q_13"),8.218406381790899e-19,"IQB U=115 V");near(get(rr,"q/e_13"),5.129525800955414,"IQB q/e");
  near(get(rr,"n_13"),5,"IQB nearest integer");near(get(rr,"Abw_13"),.129525800955414,"IQB integer deviation");
  for(unsigned i=1;i<=32;++i){const std::string key="q_"+std::to_string(i);near(get(rr,key),get(rm,key),"radius/mass equivalence");near(get(rr,key),get(rq,key),"direct-q equivalence");}
  need(rr.plots[0].points.size()==160&&rr.plots[1].points.size()==60,"32 complete bars and bounded histogram");
  double total=0;for(size_t i=1;i<rr.plots[1].points.size();i+=5)total+=rr.plots[1].points[i].second;
  near(total,32,"histogram includes all 32 measurements");need(rr.note.find("Quantisierung nicht nachgewiesen")!=std::string::npos,"no quantization assertion without uncertainties");
  auto errors=run("A_MILLIKAN_R",{2,.001,9.81,.00001,.01,2300,23,1e-6,115,1e-8,1,1e-6,197,1e-8,1});
  const double relative=std::sqrt(.03*.03+.01*.01+(.01/9.81)*(.01/9.81)+.01*.01+(1./115)*(1./115));
  near(get(errors,"sigma_q_1")/get(errors,"q_1"),relative,"Gaussian radius error, cubic radius factor");
  errors=run("A_MILLIKAN_M",{2,.001,9.81,0,0,mass,115,mass*.02,2,mass,197,0,0});
  near(get(errors,"sigma_q_1")/get(errors,"q_1"),std::hypot(.02,2./115),"mass uncertainty propagation");
  errors=run("A_MILLIKAN_Q",{2,-2*charge,.1*charge,charge,0});near(get(errors,"n_1"),-2,"signed direct charge");
  near(get(errors,"sigma_q/e_1"),.1,"charge uncertainty scaled by e");
  std::vector<double> forty={40};for(unsigned i=0;i<40;++i)forty.insert(forty.end(),{charge,0});
  const auto all=run("A_MILLIKAN_Q",forty);need(all.plots[0].points.size()==200,"40 bars");
  run("A_MILLIKAN_R",radii);run("A_MILLIKAN_M",masses);run("A_MILLIKAN_Q",qs);
  reject("A_MILLIKAN_Q",{2,charge,-1,charge,0});reject("A_MILLIKAN_M",{2,.001,9.81,0,0,mass,0,0,0,mass,100,0,0});
  reject("A_MILLIKAN_R",{2,.001,9.81,0,0,-2300,0,1e-6,100,0,0,1e-6,100,0,0});
  reject("A_MILLIKAN_R",{2,.001,9.81,0,0,2300,0,-1e-6,100,0,0,1e-6,100,0,0});
}
void planckFits(){
  // Abiheft 8b: no point for the yellow LED (590 nm, no photoemission).
  const double lambda[]={365e-9,405e-9,470e-9,525e-9},U[]={1.15,.82,.39,.10};
  std::vector<double> lu={4},le={4},fu={4},fe={4};Points energy;
  for(unsigned i=0;i<4;++i){lu.insert(lu.end(),{lambda[i],U[i]});le.insert(le.end(),{lambda[i],U[i]*charge});
    fu.insert(fu.end(),{light/lambda[i],U[i]});fe.insert(fe.end(),{light/lambda[i],U[i]*charge});energy.emplace_back(lambda[i],U[i]*charge);}
  const auto r=run("A_PLANCK_L_U",lu);near(get(r,"h"),6.718824083903723e-34,"Abiheft freely fitted h");
  near(get(r,"lambda_G"),549.1073198695331e-9,"Abiheft lambda 549 nm, not fixed-h 541 nm");
  near(get(r,"W_A"),3.6682315352519394e-19,"Abiheft work");near(get(r,"f_G"),545963324749031.06,"Abiheft threshold");
  near(get(r,"lambda_G")*get(r,"f_G"),light,"lambda/threshold inverse");
  need(has(r,"sigma_h")&&get(r,"sigma_h")>0&&has(r,"sigma_lambda_G"),"Planck statistical errors");
  near(get(r,"sigma_h"),5.916441405510105e-36,"Planck residual covariance sigma_h");
  near(get(r,"sigma_W_A"),4.136743569613753e-21,"Planck residual covariance sigma_work");
  near(get(r,"sigma_f_G"),1541946988838.9885,"threshold uncertainty includes covariance");
  near(get(r,"sigma_lambda_G"),1.5508264750411274e-9,"threshold wavelength propagation");
  need(r.plots[0].xUnit=="THz"&&r.plots[0].yUnit=="eV","Planck scaled axes");
  near(r.plots[0].points[0].first,821.3492,"THz axis");
  for(const auto& idData:std::vector<std::pair<std::string,std::vector<double>>>{{"A_PLANCK_L_E",le},{"A_PLANCK_F_U",fu},{"A_PLANCK_F_E",fe}}){
    const auto other=run(idData.first,idData.second);near(get(other,"h"),get(r,"h"),"four input variants h");near(get(other,"lambda_G"),get(r,"lambda_G"),"four input variants threshold");}
  NativeResult direct;std::string error;
  need(planckAnalysis(energy,true,direct,error),"public wavelength analysis");near(get(direct,"h"),get(r,"h"),"public legacy wavelength API");
  Fit legacy;need(fitPlanck(energy,true,legacy,error),"legacy fit");near(get(direct,"h"),legacy.h,"legacy h preserved");
  near(get(direct,"W_A"),legacy.work,"legacy work preserved");near(get(direct,"f_G"),legacy.threshold,"legacy threshold preserved");
  const auto two=run("A_PLANCK_F_U",{2,6e14,.5,8e14,1.3});need(!has(two,"sigma_h")&&two.note.find("keine Kovarianz")!=std::string::npos,"two-point Planck covariance note");
  const auto zero=run("A_PLANCK_F_U",{2,1e14,.4,2e14,.8});need(!has(zero,"lambda_G")&&zero.note.find("nicht endlich")!=std::string::npos,"zero work threshold explicitly undefined lambda");
  for(const Points& p:std::vector<Points>{{},{{1,1}},{{1,1},{1,2}},{{1,2},{2,1}},{{-1,1},{2,2}},{{1,-1},{2,2}},
      {{1,std::numeric_limits<double>::infinity()},{2,2}},Points(11,{1,1})}){
    direct=r;error.clear();need(!planckAnalysis(p,false,direct,error),"public Planck rejects invalid input");
    need(direct.values.empty()&&direct.plots.empty()&&direct.note.empty()&&!error.empty(),"public Planck safe failure");}
  run("A_PLANCK_L_U",lu);run("A_PLANCK_F_U",fu);
}
void alternatingCurrent(){
  auto r=run("A_RL",{22,si("500",.001),50./3,250,0});
  near(get(r,"Z"),56.79398540409343,"AC 8.1 impedance");near(get(r,"S"),1100.4686421512392,"AC 8.1 apparent power");
  near(get(r,"Q"),1014.5511527584778,"AC 8.1 reactive power");near(get(r,"P"),426.28299379008376,"AC 8.1 active power");
  near(get(r,"i(t)"),get(r,"I_hat"),"cos current convention");
  near(get(r,"u(t)"),get(r,"u_R(t)")+get(r,"u_L(t)")+get(r,"u_C(t)"),"component time voltages",2e-10,1e-12);
  near(get(r,"S")*get(r,"S"),get(r,"P")*get(r,"P")+get(r,"Q")*get(r,"Q"),"power triangle");
  need(r.plots.size()==2&&r.plots[0].xUnit=="V","phasor plot RL");
  const double C=si("15",1e-6);r=run("A_RLC",{22,1.2,C,50,220,.001});
  near(get(r,"I_eff"),1.3233351299603302,"AC 8.2 effective current");near(get(r,"f_0"),37.51317983987943,"AC 8.2 resonance frequency");
  near(get(r,"I_res"),10,"AC 8.2 resonance current");near(get(r,"I_hat"),std::sqrt(2.0)*get(r,"I_eff"),"peak/effective conversion");
  near(get(r,"U_hat"),std::sqrt(2.0)*220,"voltage peak/effective conversion");
  near(get(r,"Q"),get(r,"Q_L")+get(r,"Q_C"),"signed total reactive power");
  near(get(r,"u(t)"),get(r,"u_R(t)")+get(r,"u_L(t)")+get(r,"u_C(t)"),"RLC time Kirchhoff",2e-10,1e-12);
  need(r.plots.size()==3,"RLC phasor, time, resonance plots");near(r.plots[2].points[40].second,10,"resonance peak on plot");
  auto atRes=run("A_RLC",{22,1.2,C,get(r,"f_0"),220,0});near(get(atRes,"phi"),0,"phase at current resonance",0,1e-14);
  near(get(atRes,"I_eff"),10,"current at resonance");
  const auto capacitive=run("A_RLC",{22,1.2,C,10,220,0});need(get(capacitive,"phi")<0&&get(capacitive,"Q")<0,"capacitive sign");
  run("A_RLC",{22,1.2,C,50,220,.001});
  reject("A_RL",{0,1,50,230,0});reject("A_RL",{22,-1,50,230,0});reject("A_RLC",{22,1,0,50,230,0});
  reject("A_RLC",{22,1,C,0,230,0});reject("A_RLC",{22,1,C,50,-1,0});
}
void freeAndDriven(){
  auto r=run("A_FREE_SPRING",{1,4,0,.1,.2,pi/4,4});
  near(get(r,"x(t)"),.1,"undamped initial velocity contribution");near(get(r,"v(t)"),-.2,"undamped phase");near(get(r,"E(t)"),get(r,"E(0)"),"undamped energy conservation");
  r=run("A_FREE_SPRING",{1,4,2,1,0,2*pi/std::sqrt(3.0),4});
  near(get(r,"x(t)"),std::exp(-2*pi/std::sqrt(3.0)),"underdamped one damped period");near(get(r,"v(t)"),0,"underdamped full-period velocity",0,1e-15);
  need(r.note.find("Unterkritisch")!=std::string::npos,"underdamped label");
  r=run("A_FREE_SPRING",{1,4,4,1,0,.5,4});near(get(r,"x(t)"),2/std::exp(1.0),"critical displacement");
  near(get(r,"v(t)"),-2/std::exp(1.0),"critical velocity");need(r.note.find("Grenzfall")!=std::string::npos,"critical label");
  r=run("A_FREE_SPRING",{1,2,3,1,0,1,4});near(get(r,"x(t)"),2*std::exp(-1.)-std::exp(-2.),"overdamped roots");
  near(get(r,"v(t)"),-2*std::exp(-1.)+2*std::exp(-2.),"overdamped velocity");need(r.note.find("Kriechfall")!=std::string::npos,"overdamped label");
  for(double R:{0.,2.,4.,6.}){r=run("A_FREE_RLC",{1,.25,R,.01,-.02,0,3});
    near(get(r,"q(t)"),.01,"RLC initial charge");near(get(r,"I(t)"),-.02,"RLC initial current");
    near(get(r,"U_L(t)")+R*get(r,"I(t)")+get(r,"U_C(t)"),0,"free RLC Kirchhoff",0,1e-14);}
  r=run("A_FREE_RLC",{1,.25,4,1,0,.5,4});near(get(r,"q(t)"),2/std::exp(1.0),"RLC critical charge");near(get(r,"I(t)"),-2/std::exp(1.0),"RLC critical current");
  r=run("A_FREE_RLC",{1,.5,3,1,0,1,4});near(get(r,"q(t)"),2*std::exp(-1.)-std::exp(-2.),"RLC overdamped charge");
  r=run("A_FREE_RLC",{1,.25,2,1,0,2*pi/std::sqrt(3.0),4});near(get(r,"q(t)"),std::exp(-2*pi/std::sqrt(3.0)),"RLC underdamped charge");
  need(get(r,"E(t)")<get(r,"E(0)"),"damping loses energy");
  r=run("A_FREE_RLC",{1,.0001,1,.001,0,.01,10});
  need(r.plots[0].points.size()==400&&r.note.find("400 Punkte")!=std::string::npos,"bounded high frequency graph and resolution warning");
  const double h=1e-4;
  const auto before=run("A_FREE_SPRING",{1,2,3,1,.2,1-h,4});
  const auto after=run("A_FREE_SPRING",{1,2,3,1,.2,1+h,4});
  r=run("A_FREE_SPRING",{1,2,3,1,.2,1,4});
  near((get(after,"x(t)")-get(before,"x(t)"))/(2*h),get(r,"v(t)"),"independent numerical derivative",1e-7);
  near((get(after,"v(t)")-get(before,"v(t)"))/(2*h),get(r,"a(t)"),"independent ODE acceleration",1e-7);
  r=run("A_DRIVEN_SPRING",{1,4,.4,1,1/pi,1});near(get(r,"Amplitude"),1.25,"driven amplitude at omega0");
  near(get(r,"phi"),pi/2,"lag phase at omega0");near(get(r,"f_Amplitude_max"),std::sqrt(3.92)/(2*pi),"amplitude maximum below f0");
  need(get(r,"f_Amplitude_max")<get(r,"f_0"),"distinguish mechanical amplitude and electrical current resonance");
  const auto peak=run("A_DRIVEN_SPRING",{1,4,.4,1,get(r,"f_Amplitude_max"),1});
  near(get(peak,"Amplitude"),get(r,"Amplitude_max"),"resonance inverse frequency evaluation");
  r=run("A_DRIVEN_SPRING",{1,4,4,1,0,1});near(get(r,"f_Amplitude_max"),0,"strong damping max at zero");near(get(r,"Amplitude_max"),.25,"static compliance");
  reject("A_FREE_SPRING",{0,4,1,1,0,1,3});reject("A_FREE_SPRING",{1,4,-1,1,0,1,3});
  reject("A_FREE_RLC",{1,0,1,1,0,1,3});reject("A_FREE_RLC",{1,.25,1,1,0,-1,3});
  reject("A_DRIVEN_SPRING",{1,4,0,1,.3,1});reject("A_DRIVEN_SPRING",{1,4,.4,-1,.3,1});
}
void publicSafety(){
  need(examples.size()==26,"every ID exercised successfully");reject("",{});reject("A_UNKNOWN",{1,2});reject("A_RLC_extra",{22,1,1,50,220,0});
  for(const auto& category:analysisCategories())for(const auto& w:analysisCatalog(category.id)){
    const auto& id=w.calculation.id;const auto& seed=examples.at(id);reject(id,{});
    auto v=seed;v.push_back(0);reject(id,v);v=seed;v.pop_back();reject(id,v);
    for(size_t i=0;i<seed.size();++i)for(double invalid:{std::numeric_limits<double>::infinity(),-std::numeric_limits<double>::infinity(),std::numeric_limits<double>::quiet_NaN()}){
      v=seed;v[i]=invalid;reject(id,v);}
    if(w.maxCount)for(double invalid:{-1.,0.,1.,1.5,static_cast<double>(w.maxCount)+1,1e300}){v=seed;v[0]=invalid;reject(id,v);}
    // Finite extremes must either fail cleanly or produce only finite bounded results.
    for(size_t i=w.maxCount?1:0;i<seed.size();++i){v=seed;v[i]=1e308;NativeResult r;std::string error;
      if(analysisCalculate(id,v,r,error)){need(error.empty(),"extreme success clears error");bounded(r);}
      else need(r.values.empty()&&r.note.empty()&&r.plots.empty()&&!error.empty(),"finite overflow safe failure");}
  }
}
} // namespace
int main(){
  catalogs();diagrams();fits();millikanBatch();planckFits();alternatingCurrent();freeAndDriven();publicSafety();
  std::cout<<"PASS ANALYSIS: "<<checks<<" checks; 5 categories; 26 workflows (4 graphs, 10 fits, 3 Millikan, 4 Planck, 5 AC/oscillation); all 32 IQB measurements; original AC 8.1/8.2 and Abiheft 8b\n";
}

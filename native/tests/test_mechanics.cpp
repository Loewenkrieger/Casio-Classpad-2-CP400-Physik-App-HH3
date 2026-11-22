// SPDX-License-Identifier: GPL-3.0-or-later
#include "extensions.hpp"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <set>

using namespace physik;
namespace {
unsigned checks=0,failures=0;
void check(bool condition,const std::string& label){
  ++checks;
  if(!condition){++failures;std::fprintf(stderr,"FAIL %s\n",label.c_str());}
}
void near(double actual,double expected,const std::string& label,double tolerance=1e-9){
  const bool okay=std::isfinite(actual)&&std::isfinite(expected)&&
    std::abs(actual-expected)<=tolerance*std::max(1.0,std::abs(expected));
  check(okay,label);
  if(!okay)std::fprintf(stderr,"  actual=%.17g expected=%.17g\n",actual,expected);
}
bool has(const NativeResult& r,const std::string& name){
  for(const auto& v:r.values)if(v.name==name)return true;
  return false;
}
double value(const NativeResult& r,const std::string& name){
  for(const auto& v:r.values)if(v.name==name)return v.value;
  check(false,"missing result "+name);return std::numeric_limits<double>::quiet_NaN();
}
void finite(const NativeResult& r){
  check(!r.values.empty(),"nonempty result");
  for(const auto& v:r.values)check(!v.name.empty()&&std::isfinite(v.value),"finite named result "+v.name);
  for(const auto& p:r.plots){
    check(!p.title.empty()&&!p.xLabel.empty()&&!p.yLabel.empty(),"plot labels");
    check(p.points.size()<=400&&p.model.size()<=400,"plot limit");
    for(const auto& points:{&p.points,&p.model})for(const auto& q:*points)
      check(std::isfinite(q.first)&&std::isfinite(q.second),"finite plot point");
  }
}
NativeResult run(const std::string& id,const std::vector<double>& z){
  NativeResult r;r.values.push_back({"stale",42,"1"});r.note="stale";
  std::string error="stale";
  const bool okay=mechanicsCalculate(id,z,r,error);
  check(okay,id+" success: "+error);
  check(error.empty(),id+" clears error on success");
  check(!has(r,"stale")&&r.note!="stale",id+" replaces result");
  if(okay)finite(r);
  return r;
}
void reject(const std::string& id,const std::vector<double>& z){
  NativeResult r;r.values.push_back({"stale",42,"1"});r.note="stale";
  r.plots.push_back({"old","t","s","x","m",{{0,1}},{},true});
  std::string error="stale";
  check(!mechanicsCalculate(id,z,r,error),id+" must reject");
  check(r.values.empty()&&r.note.empty()&&r.plots.empty(),id+" failure clears entire result");
  check(!error.empty()&&error!="stale",id+" reports error");
}
double scale(const std::string& unit){
  if(unit=="km/h")return 1/3.6;
  if(unit=="deg")return pi/180;
  if(unit=="g")return 0.001;
  return 1;
}
double number(const std::string& text){
  char* end=nullptr;
  const double n=std::strtod(text.c_str(),&end);
  check(end!=text.c_str()&&*end=='\0'&&std::isfinite(n),"numeric default");return n;
}
std::vector<double> defaults(const Workflow& w){
  std::vector<double> z;
  for(std::size_t i=0;i<w.defaults.size();++i)z.push_back(number(w.defaults[i])*scale(w.calculation.parameters[i].defaultUnit));
  if(w.maxCount)for(unsigned n=0;n<static_cast<unsigned>(z[0]);++n)
    for(std::size_t j=0;j<w.repeated.size();++j)z.push_back(number(w.repeatedDefaults[j])*scale(w.repeated[j].defaultUnit));
  return z;
}

void catalogue(){
  const auto categories=mechanicsCategories();
  check(categories.size()==8,"8 categories");
  const unsigned counts[]={7,4,2,4,7,2,2,10};
  std::set<std::string> categoryIds,ids;
  const double nan=std::numeric_limits<double>::quiet_NaN(),inf=std::numeric_limits<double>::infinity();
  for(std::size_t c=0;c<categories.size();++c){
    const auto& category=categories[c];
    check(categoryIds.insert(category.id).second&&!category.title.empty(),"category identity");
    const auto catalog=mechanicsCatalog(category.id);
    check(catalog.size()==counts[c],"category count "+category.id);
    for(const auto& w:catalog){
      const auto& calculation=w.calculation;
      check(calculation.id.compare(0,2,"M_")==0&&ids.insert(calculation.id).second,"unique stable M_ id");
      check(!calculation.title.empty()&&!calculation.formula.empty(),"workflow description");
      check(calculation.parameters.size()==w.defaults.size(),"base/default arity");
      check(w.repeated.size()==w.repeatedDefaults.size(),"repeated/default arity");
      check(w.minCount<=w.maxCount&&w.maxCount<=40,"repeat limits");
      for(const auto* parameters:{&calculation.parameters,&w.repeated})for(const auto& parameter:*parameters){
        check(!parameter.name.empty(),"parameter name");
        if(category.id=="M_STRASSE"&&parameter.unit=="1")
          check(parameter.name=="Reibungszahl mu","neutral braking friction label");
        if(parameter.unit=="rad")check(parameter.defaultUnit=="deg","angle in rad, default deg");
        if(parameter.name=="g")near(number(w.defaults[&parameter-calculation.parameters.data()]),9.81,"visible g default");
      }
      const auto z=defaults(w);
      run(calculation.id,z);
      reject(calculation.id,{});
      auto bad=z;bad.pop_back();reject(calculation.id,bad);
      bad=z;bad.push_back(0);reject(calculation.id,bad);
      for(std::size_t i=0;i<z.size();++i){
        bad=z;bad[i]=nan;reject(calculation.id,bad);
        bad=z;bad[i]=inf;reject(calculation.id,bad);
        bad=z;bad[i]=-inf;reject(calculation.id,bad);
      }
      if(w.maxCount)for(double count:{-1.0,0.0,0.5,1.5,41.0,1e300}){bad=z;bad[0]=count;reject(calculation.id,bad);}
    }
  }
  check(ids.size()==38,"38 workflows");
  check(mechanicsCatalog("").empty()&&mechanicsCatalog("M_UNKNOWN").empty(),"unknown category empty");
  reject("",{});reject("M_UNKNOWN",{});reject("KIN_STATE",{0,1,2,3});
}

// Acceptance fixtures transcribed from the approved school-material handoff.
void originalProblems(){
  auto r=run("M_TARGET_V",{6,2.5,0,0,9.81});
  near(value(r,"v0"),8.404284622,"treehouse launch speed");
  near(value(r,"t_Ziel"),0.71392156,"treehouse time",1e-8);
  near(value(r,"Reichweite"),6,"treehouse range");
  r=run("M_OBLIQUE",{0,100/3.6,30*pi/180,9.81});
  near(value(r,"Reichweite"),68.117174131,"100 km/h 30 degrees");
  r=run("M_PROJECTILE_STATE",{0,0,100/3.6,30*pi/180,9.81,1});
  near(value(r,"x"),24.056261216,"100 km/h 30 degrees x at exactly 1 s");
  near(value(r,"y"),8.983888889,"100 km/h 30 degrees y at exactly 1 s");
  near(value(r,"vy"),4.078888888889,"100 km/h vertical velocity at 1 s");
  near(value(r,"Reichweite"),68.117174131,"state mask descending flight end");
  r=run("M_TARGET_V",{68.117174131,0,0,pi/6,1.62});
  near(value(r,"v0"),11.288091025,"Moon same range");
  r=run("M_BRAKE_MU",{80/3.6,1,0.5,9.81,9});
  near(value(r,"s_Anhalten"),72.561382,"road stopping distance",1e-8);
  near(value(r,"Hindernisphase"),1,"obstacle during reaction");
  near(value(r,"t_Hindernis"),0.405,"obstacle reaction impact time");
  near(value(r,"v_Hindernis"),80/3.6,"reaction impact retains speed");
  r=run("M_PASSING",{1,100/3.6,60/3.6,4.5,15,40,40,30,0,30,0});
  near(value(r,"Relativweg"),99.5,"passing lengths and gaps");
  near(value(r,"t"),8.955,"passing duration");
  near(value(r,"xA_1")-value(r,"xB_1"),44.5,"passing front positions");
  r=run("M_RIVER",{90,1.5,4,0});
  near(value(r,"t_Ueberfahrt"),22.5,"ferry time");near(value(r,"Abdrift"),33.75,"ferry drift");
  r=run("M_WORK_FRICTION",{50,25});near(value(r,"Q"),1250,"halfpipe heat");
  near(value(r,"W_Reibung"),-1250,"signed friction work");
  r=run("M_ENERGY_V",{60,0,8,2,0,1250,9.81});
  near(value(r,"v2"),8.721,"halfpipe launch",1e-4);
  near(value(r,"h_max_ideal"),5.8763,"halfpipe maximum height",1e-5);
  near(value(r,"E2"),value(r,"E1")-1250,"halfpipe energy balance");
  r=run("M_WORK_ANGLE",{500,200,pi/6,300});
  near(value(r,"W"),86602.54037844386,"lift work");near(value(r,"P"),288.6751345948129,"lift power");
  r=run("M_ELASTIC",{0.040,0.060,4,0});
  near(value(r,"v1_nach"),-0.8,"elastic signed v1");near(value(r,"v2_nach"),3.2,"elastic v2");
  near(value(r,"Delta_K"),0,"elastic delta K");
  r=run("M_INELASTIC",{0.040,0.060,4,0});
  near(value(r,"v1_nach"),1.6,"inelastic v1");near(value(r,"v2_nach"),1.6,"inelastic v2");
  near(value(r,"Delta_K"),-0.192,"inelastic energy change");
  r=run("M_INCLINE_H",{80,100,200,0.1,0.05,0,9.81});
  near(value(r,"a"),4.48021,"skier acceleration",2e-6);
  near(value(r,"v_Ende"),42.333,"skier speed",2e-5);
  near(value(r,"N"),80*9.81*std::sqrt(3.0)/2,"skier normal force");
  r=run("M_CIRCLE_V",{10,30/3.6,60});
  near(value(r,"F_z"),416.6666666667,"circle force");near(value(r,"omega"),0.8333333333333,"circle omega");
}

void projectileTests(){
  auto r=run("M_FREE",{20,9.81});
  near(value(r,"t_Flug"),std::sqrt(40/9.81),"free fall time");
  near(value(r,"v_Aufprall"),std::sqrt(40*9.81),"free fall speed");
  r=run("M_VERTICAL",{0,10,9.81});
  near(value(r,"t_Flug"),20/9.81,"vertical ground launch");near(value(r,"h_max"),50/9.81,"vertical peak");
  r=run("M_VERTICAL",{10,-5,9.81});
  near(value(r,"h_max"),10,"downward throw maximum is initial height");
  near(value(r,"t_Flug"),(std::sqrt(25+196.2)-5)/9.81,"downward flight");
  r=run("M_OBLIQUE",{0,10,pi/2,9.81});near(value(r,"Reichweite"),0,"exact vertical oblique limit");
  r=run("M_FREE",{0,9.81});near(value(r,"t_Flug"),0,"zero height free fall");
  r=run("M_TARGET_V",{20,0,5,pi/4,9.81});
  const double t=value(r,"t_Ziel");
  near(value(r,"vy0")*t-9.81*t*t/2,5,"raised target vertical equation");
  near(value(r,"vx0")*t,20,"raised target horizontal equation");
  for(double h:{0.0,0.1,2.5,100.0})for(double v:{0.0,2.0,20.0})for(double angle:{-0.7,0.0,0.9}){
    r=run("M_OBLIQUE",{h,v,angle,9.81});
    const double time=value(r,"t_Flug");
    near(h+v*std::sin(angle)*time-9.81*time*time/2,0,"projectile ground equation",1e-8);
    near(value(r,"v_Aufprall")*value(r,"v_Aufprall"),v*v+2*9.81*h,"projectile energy");
    check(time>=0,"projectile nonnegative flight");
    if(v>0&&angle>0&&value(r,"Reichweite")>0){
      const auto inverse=run("M_TARGET_V",{value(r,"Reichweite"),h,0,angle,9.81});
      near(value(inverse,"v0"),v,"projectile inverse roundtrip");
    }
  }
  reject("M_FREE",{-1,9.81});reject("M_FREE",{1,0});reject("M_FREE",{1,-9.81});
  reject("M_HORIZONTAL",{1,-1,9.81});reject("M_OBLIQUE",{1,2,pi,9.81});
  reject("M_TARGET_V",{0,1,0,0,9.81});reject("M_TARGET_V",{10,0,1,0,9.81});
  reject("M_TARGET_V",{10,0,0,0,9.81});reject("M_TARGET_V",{10,0,0,pi/2,9.81});
  reject("M_TARGET_V",{10,-1,0,pi/4,9.81});
}

void projectileHeightTests(){
  // Independent construction: vx0=15, vy0=20, g=10, y=-7+20*t-5*t^2.
  // The raised target y=8 is crossed at t=1 and t=3; the apex is (30,13).
  const double angle=std::atan2(4.0,3.0);
  auto r=run("M_PROJECTILE_HEIGHT",{-7,8,25,angle,10});
  near(value(r,"vx0"),15,"raised target initial vx");near(value(r,"vy0"),20,"raised target initial vy");
  near(value(r,"t_Ziel_aufsteigend"),1,"raised target ascending time");
  near(value(r,"t_Ziel_absteigend"),3,"raised target descending time");
  near(value(r,"t_Flug"),3,"flight ends at descending raised target, not y=0");
  near(value(r,"Zieltreffer"),2,"two future height crossings");
  near(value(r,"Reichweite"),45,"raised target range");near(value(r,"h_max"),13,"raised target apex");
  near(value(r,"vy_Ziel"),-10,"descending target vy");
  near(value(r,"v_Ziel"),std::sqrt(325.0),"descending target speed");
  near(value(r,"alpha_Ziel"),std::atan2(-10.0,15.0),"descending target angle");
  check(r.plots.size()==1&&r.plots[0].points.size()==65,"raised-target finite plot");
  near(r.plots[0].points.back().first,45,"plot ends at target range");
  near(r.plots[0].points.back().second,8,"plot ends at raised target, not zero");
  const double end=value(r,"t_Flug"),ascending=value(r,"t_Ziel_aufsteigend");
  for(double t:{0.0,0.25,0.5,1.0,1.5,2.0,2.5,ascending,end}){
    const auto state=run("M_PROJECTILE_STATE",{-7,8,25,angle,10,t});
    check(value(state,"t")==t,"state uses exact input t without sampling or clamping");
    near(value(state,"x"),15*t,"independent exact-time x");
    near(value(state,"y"),-7+20*t-5*t*t,"independent exact-time y");
    near(value(state,"vx"),15,"independent exact-time vx");near(value(state,"vy"),20-10*t,"independent exact-time vy");
    near(value(state,"v"),std::hypot(15.0,20-10*t),"independent exact-time speed");
    near(value(state,"alpha"),std::atan2(20-10*t,15.0),"independent exact-time direction");
    near(value(state,"t_Flug"),3,"state retains descending flight end");
  }
  reject("M_PROJECTILE_STATE",{-7,8,25,angle,10,-0.001});
  reject("M_PROJECTILE_STATE",{-7,8,25,angle,10,std::nextafter(0.0,-1.0)});
  reject("M_PROJECTILE_STATE",{-7,8,25,angle,10,std::nextafter(end,std::numeric_limits<double>::infinity())});
  reject("M_PROJECTILE_STATE",{-7,8,25,angle,10,4});

  // A shift of the height origin must not change any time or velocity.
  for(double shift:{-1000000.0,-100.0,0.0,100.0,1000000.0}){
    r=run("M_PROJECTILE_HEIGHT",{-7+shift,8+shift,25,angle,10});
    near(value(r,"t_Ziel_aufsteigend"),1,"height-origin invariant ascent");
    near(value(r,"t_Flug"),3,"height-origin invariant flight end");
    near(value(r,"h_max"),13+shift,"shifted apex");near(value(r,"vy_Ziel"),-10,"height-origin invariant target velocity");
    const auto state=run("M_PROJECTILE_STATE",{-7+shift,8+shift,25,angle,10,1.5});
    near(value(state,"x"),22.5,"height-origin invariant x");
    near(value(state,"y")-shift,11.75,"negative or positive height origin state");
    near(value(state,"vy"),5,"height-origin invariant state vy");
  }
  r=run("M_PROJECTILE_HEIGHT",{-7,-12,25,angle,10});
  near(value(r,"t_Flug"),2+std::sqrt(5.0),"lower negative target descending time");
  near(value(r,"Aufsteigender_Treffer"),0,"past ascent is not a physical crossing");
  near(value(r,"Zieltreffer"),1,"only one future crossing for lower target");
  check(!has(r,"t_Ziel_aufsteigend")&&r.note.find("vor dem Start")!=std::string::npos,"past height time explicitly excluded");
  r=run("M_PROJECTILE_HEIGHT",{-7,-12,25,-angle,10});
  near(value(r,"t_Flug"),std::sqrt(5.0)-2,"downward throw reaches lower negative target");
  check(value(r,"vy_Ziel")<0,"downward target is descending");
  r=run("M_PROJECTILE_HEIGHT",{-7,-7,25,angle,10});
  near(value(r,"t_Ziel_aufsteigend"),0,"same-level ascent at launch");near(value(r,"t_Flug"),4,"same-level descending end");
  r=run("M_PROJECTILE_STATE",{-7,-7,25,-angle,10,0});
  near(value(r,"t_Flug"),0,"downward same-height flight is instantaneous");near(value(r,"y"),-7,"instantaneous target height");
  reject("M_PROJECTILE_STATE",{-7,-7,25,-angle,10,0.001});
  r=run("M_PROJECTILE_HEIGHT",{-7,-12,0,0,10});
  near(value(r,"t_Flug"),1,"free fall between negative heights");near(value(r,"vy_Ziel"),-10,"negative-height free fall target velocity");

  r=run("M_PROJECTILE_HEIGHT",{-7,13,25,angle,10});
  near(value(r,"t_Ziel_aufsteigend"),2,"apex ascent branch");near(value(r,"t_Ziel_absteigend"),2,"apex descent branch");
  near(value(r,"Zieltreffer"),1,"apex touching counts once");
  check(r.note.find("Scheitelberuehrung")!=std::string::npos,"coincident height times labelled");
  r=run("M_PROJECTILE_STATE",{-7,13,25,angle,10,value(r,"t_Flug")});
  near(value(r,"y"),13,"apex target state");near(value(r,"vy"),0,"apex vertical velocity");
  r=run("M_PROJECTILE_STATE",{-7,13,20,pi/2,10,2});
  near(value(r,"x"),0,"vertical raised-target throw");near(value(r,"v"),0,"rest at vertical apex");
  check(r.note.find("Richtung nicht definiert")!=std::string::npos,"zero apex direction convention");
  for(double g:{1.62,9.81,20.0}){
    // At h0+3*vy0^2/(8*g), the two roots are vy0/(2*g) and 3*vy0/(2*g).
    const double target=-30+150/g;
    r=run("M_PROJECTILE_HEIGHT",{-30,target,25,angle,g});
    near(value(r,"t_Ziel_aufsteigend"),10/g,"changed-g raised-target ascent");
    near(value(r,"t_Flug"),30/g,"changed-g raised-target descent");near(value(r,"vy_Ziel"),-10,"changed-g raised-target velocity");
  }
  reject("M_PROJECTILE_HEIGHT",{-7,13.01,25,angle,10});
  reject("M_PROJECTILE_HEIGHT",{-7,8,25,-angle,10});reject("M_PROJECTILE_HEIGHT",{-7,8,0,0,10});
  reject("M_PROJECTILE_STATE",{-7,13.01,25,angle,10,0});
  reject("M_PROJECTILE_HEIGHT",{-7,-12,-25,angle,10});reject("M_PROJECTILE_HEIGHT",{-7,-12,25,pi,10});
  reject("M_PROJECTILE_HEIGHT",{-7,-12,25,angle,0});reject("M_PROJECTILE_STATE",{-7,-12,25,angle,-10,1});
  const double huge=std::numeric_limits<double>::max();
  reject("M_PROJECTILE_HEIGHT",{huge,-huge,25,angle,10});
  reject("M_PROJECTILE_HEIGHT",{0,0,huge,pi/2,1});
}

void brakingTests(){
  const double v=80/3.6,a=4.905,stop=v+v*v/(2*a);
  auto r=run("M_BRAKE_A",{v,1,a,40});
  near(value(r,"Hindernisphase"),2,"impact while braking");
  near(value(r,"v_Hindernis"),std::sqrt(v*v-2*a*(40-v)),"braking impact speed");
  near(value(r,"t_Hindernis"),1+(v-value(r,"v_Hindernis"))/a,"braking impact time");
  r=run("M_BRAKE_A",{v,1,a,stop});near(value(r,"Hindernisphase"),0,"at stopping boundary");
  check(!has(r,"v_Hindernis"),"no fake impact when stopped");
  r=run("M_BRAKE_A",{0,1,a,0});near(value(r,"t_Anhalten"),0,"initially stopped");
  r=run("M_BRAKE_A",{v,0,a,0});near(value(r,"Hindernisphase"),2,"immediate brake obstacle");
  near(value(r,"t_Hindernis"),0,"immediate impact time");
  for(double speed:{1.0,10.0,80/3.6,40.0})for(double reaction:{0.0,1.0,2.0}){
    const double distance=speed*reaction+speed*speed/(2*a);
    r=run("M_SAFE_V",{distance,reaction,0.5,9.81});near(value(r,"v_sicher"),speed,"safe speed inverse");
    r=run("M_REQUIRED_A",{speed,reaction,distance,9.81});
    near(value(r,"a_Betrag"),a,"braking inverse a");near(value(r,"mu_erforderlich"),0.5,"required friction inverse");
  }
  r=run("M_SAFE_V",{0,0,0.5,9.81});near(value(r,"v_sicher"),0,"zero safe speed");
  reject("M_BRAKE_A",{10,-1,5,100});reject("M_BRAKE_A",{10,1,0,100});
  reject("M_BRAKE_A",{10,1,5,-1});reject("M_BRAKE_MU",{10,1,-0.5,9.81,100});
  reject("M_SAFE_V",{-1,1,0.5,9.81});reject("M_SAFE_V",{10,1,0,9.81});
  reject("M_REQUIRED_A",{10,1,10,9.81});reject("M_REQUIRED_A",{10,1,9,9.81});
}

void encounterTests(){
  auto r=run("M_ENCOUNTER",{1,0,10,100,0,30,0,30,0});
  near(value(r,"t"),10,"uniform encounter");near(value(r,"xA_1"),100,"encounter position");
  r=run("M_ENCOUNTER",{1,8,-6,0,0,10,2,10,0});
  near(value(r,"Loesungen"),2,"two valid quadratic roots");near(value(r,"t_1"),2,"first encounter");near(value(r,"t_2"),4,"second encounter");
  r=run("M_ENCOUNTER",{1,9,-6,0,0,10,2,10,0});
  near(value(r,"Loesungen"),1,"tangent root counted once");near(value(r,"t"),3,"tangent time");
  r=run("M_ENCOUNTER",{1,8,-6,0,0,3,2,10,0});
  near(value(r,"Loesungen"),1,"reject root outside body A horizon");near(value(r,"t"),2,"remaining finite-phase root");
  r=run("M_ENCOUNTER",{2,0,0,2,0,2,1,1,0,2,0,3,0});
  near(value(r,"t"),2,"root at asynchronous phase boundary");near(value(r,"Loesungen"),1,"boundary root not duplicated");
  r=run("M_ENCOUNTER",{2,0,10,100,0,20,0,10,0,0,0,20,0});
  near(value(r,"t"),10,"one body has zero-duration second phase");near(value(r,"Gueltig_bis"),20,"minimum total phase duration");
  r=run("M_ENCOUNTER",{1,0,-1,0,1,2,0,2,0});near(value(r,"t"),0,"initial contact root");
  r=run("M_PASSING",{1,10,20,4.5,15,40,40,30,2,30,0});
  near(value(r,"t"),(10+std::sqrt(498.0))/2,"accelerating passer may start slower");
  reject("M_ENCOUNTER",{1,0,10,100,0,5,0,20,0});
  reject("M_ENCOUNTER",{1,10,-6,0,0,10,2,10,0});
  reject("M_ENCOUNTER",{1,0,1,0,1,10,0,10,0});
  reject("M_ENCOUNTER",{1,0,1,10,0,0,0,10,0});
  reject("M_ENCOUNTER",{2,0,1,10,0,10,0,10,0,-1,0,10,0});
  reject("M_PASSING",{1,10,20,4,10,5,5,20,0,20,0});
  reject("M_PASSING",{1,20,10,0,10,5,5,20,0,20,0});
  reject("M_PASSING",{1,20,10,4,10,5,5,20,-2,20,0});
  reject("M_PASSING",{1,20,10,4,10,5,5,0.1,0,0.1,0});
}

void vectorTests(){
  auto r=run("M_COMPONENTS",{10,pi/6});near(value(r,"vx"),5*std::sqrt(3.0),"velocity x component");near(value(r,"vy"),5,"velocity y component");
  r=run("M_VECTOR_SUM",{2,3,0,4,pi/2});
  near(value(r,"vx"),3,"vector sum vx");near(value(r,"vy"),4,"vector sum vy");near(value(r,"v"),5,"vector sum norm");
  near(value(r,"alpha"),std::atan2(4.0,3.0),"vector direction");
  r=run("M_VECTOR_SUM",{1,0,0});near(value(r,"v"),0,"zero vector convention");
  std::vector<double> many{40};for(unsigned i=0;i<40;++i){many.push_back(1);many.push_back(0);}
  r=run("M_VECTOR_SUM",many);near(value(r,"vx"),40,"40 vectors");check(r.plots[0].points.size()==41,"vector plot size");
  for(double stream:{-1.5,0.0,1.5,3.9}){
    r=run("M_RIVER_STEER",{90,stream,4});
    near(value(r,"Abdrift"),0,"opposite-bank steering has zero drift");
    near(stream+4*std::sin(value(r,"Steuerwinkel")),0,"upstream steering compensation");
    near(value(r,"t_Ueberfahrt"),90/std::sqrt(16-stream*stream),"steered crossing time");
  }
  r=run("M_RIVER",{90,-1.5,4,pi/6});near(value(r,"vx"),0.5,"river angled heading");
  reject("M_COMPONENTS",{-1,0});reject("M_VECTOR_SUM",{1,-1,0});
  reject("M_VECTOR_SUM",{2,1,0});reject("M_RIVER",{90,1.5,4,pi/2});
  reject("M_RIVER_STEER",{90,4,4});reject("M_RIVER_STEER",{90,-5,4});reject("M_RIVER_STEER",{0,1,4});
}

void energyTests(){
  auto r=run("M_WORK_ANGLE",{100,2,pi,4});near(value(r,"W"),-200,"signed opposing work");
  r=run("M_WORK_LIFT",{60,-2,9.81,10});near(value(r,"W"),-1177.2,"negative lifting work");
  r=run("M_WORK_SPRING",{100,-0.2,0.1});near(value(r,"E1"),2,"spring energy 1");near(value(r,"E2"),0.5,"spring energy 2");
  near(value(r,"W_Feder"),1.5,"spring work");near(value(r,"W_aussen"),-1.5,"spring external work");
  for(double mass:{0.1,2.0,60.0})for(double work:{-1.0,0.0,20.0}){
    const double h1=8,h2=2,v1=3,heat=mass*2;
    r=run("M_ENERGY_V",{mass,v1,h1,h2,work,heat,9.81});const double v2=value(r,"v2");
    auto inverse=run("M_ENERGY_H",{mass,v1,h1,v2,work,heat,9.81});near(value(inverse,"h2"),h2,"energy height inverse");
    inverse=run("M_ENERGY_LOSS",{mass,v1,h1,v2,h2,work,9.81});near(value(inverse,"Q"),heat,"energy loss inverse");
    near(value(r,"E2")+heat,value(r,"E1")+work,"energy conservation");
  }
  r=run("M_ENERGY_H",{60,0,8,0,0,1250,9.81});near(value(r,"h2"),8-1250/(60*9.81),"halfpipe maximum inverse");
  for(double h:{0.1,0.3,1.0,3.0,8.0,100.0}){
    const auto state=run("M_ENERGY_V",{0.3,0,h,0,0,0,9.81});
    const auto loss=run("M_ENERGY_LOSS",{0.3,0,h,value(state,"v2"),0,0,9.81});
    near(value(loss,"Q"),0,"zero-loss inverse tolerates arithmetic roundoff");
  }
  r=run("M_ENERGY_V",{1,0,-2,-3,0,0,9.81});near(value(r,"v2"),std::sqrt(19.62),"negative height reference allowed");
  reject("M_WORK_ANGLE",{1,2,0,0});reject("M_WORK_LIFT",{-1,2,9.81,10});reject("M_WORK_FRICTION",{-1,2});
  reject("M_WORK_SPRING",{0,1,0});reject("M_ENERGY_V",{1,0,0,1,0,0,9.81});
  reject("M_ENERGY_V",{1,1,0,0,0,-1,9.81});reject("M_ENERGY_H",{0,1,1,1,0,0,9.81});
  reject("M_ENERGY_LOSS",{1,0,0,10,0,0,9.81});
}

void collisionTests(){
  for(double m1:{0.04,1.0,5.0})for(double m2:{0.06,1.0,3.0})for(double u1:{-4.0,0.0,4.0})for(double u2:{-2.0,0.0,2.0}){
    for(const std::string id:{"M_ELASTIC","M_INELASTIC"}){
      const auto r=run(id,{m1,m2,u1,u2});
      near(value(r,"p_nach"),m1*u1+m2*u2,"collision momentum conservation");
      if(id=="M_ELASTIC"){
        near(value(r,"Delta_K"),0,"elastic energy conservation");
        near(value(r,"v2_nach")-value(r,"v1_nach"),u1-u2,"relative velocity reversal");
      }else{
        near(value(r,"v1_nach"),value(r,"v2_nach"),"inelastic common velocity");
        near(value(r,"Delta_K"),-0.5*m1*m2/(m1+m2)*(u1-u2)*(u1-u2),"inelastic loss formula");
      }
    }
  }
  reject("M_ELASTIC",{0,1,1,0});reject("M_INELASTIC",{1,-1,1,0});
}

void inclineTests(){
  auto r=run("M_INCLINE",{80,pi/6,0.1,0.05,0,200,9.81});
  near(value(r,"a"),9.81*(0.5-0.05*std::sqrt(3.0)/2),"downward kinetic friction");
  r=run("M_INCLINE",{10,0.1,0.2,0.1,0,10,9.81});
  near(value(r,"Haftet"),1,"static friction holds");near(value(r,"a"),0,"static acceleration");
  near(value(r,"F_R"),-value(r,"F_H"),"static friction balances slope");
  near(value(r,"Weg_erreicht"),0,"static body cannot reach requested distance");check(!has(r,"v_Ende")&&!has(r,"t"),"no invented end-state for static body");
  r=run("M_INCLINE",{10,0.1,0.2,0.1,1,2,9.81});near(value(r,"Haftet"),0,"moving body uses kinetic friction");
  r=run("M_INCLINE",{10,pi/6,0.2,0.1,-10,1,9.81});
  check(value(r,"F_R")>0&&value(r,"a")>0&&value(r,"v_Ende")<0,"upward motion friction points down");
  near(value(r,"v_Ende"),-std::sqrt(100-2*9.81*(0.5+0.1*std::sqrt(3.0)/2)),"upward finite-distance speed");
  r=run("M_INCLINE",{10,pi/6,0.2,0.1,-10,100,9.81});
  near(value(r,"Weg_erreicht"),0,"upward path ends before reversal");check(!has(r,"v_Ende"),"no square root after stopping");
  near(value(r,"s_Stillstand"),100/(2*value(r,"a")),"upward stopping distance");
  reject("M_INCLINE",{10,pi/2,0.2,0.1,1,10,9.81});reject("M_INCLINE",{10,0.1,0.1,0.2,1,10,9.81});
  reject("M_INCLINE_H",{80,201,200,0.1,0.05,0,9.81});reject("M_INCLINE_H",{80,100,0,0.1,0.05,0,9.81});
}

void circularTests(){
  for(double radius:{0.5,10.0,50.0})for(double speed:{1.0,30/3.6,20.0}){
    const double omega=speed/radius,mass=60;
    for(const auto& input:std::vector<std::pair<std::string,std::vector<double>>>{
        {"M_CIRCLE_V",{radius,speed,mass}},{"M_CIRCLE_OMEGA",{radius,omega,mass}},
        {"M_CIRCLE_F",{radius,omega/(2*pi),mass}},{"M_CIRCLE_T",{radius,2*pi/omega,mass}},
        {"M_CIRCLE_R",{speed,omega,mass}},{"M_CIRCLE_FORCE_R",{mass,speed,mass*speed*speed/radius}}}){
      const auto r=run(input.first,input.second);
      near(value(r,"r"),radius,"circle inverse radius");near(value(r,"v"),speed,"circle speed");
      near(value(r,"omega"),omega,"circle omega");near(value(r,"f")*value(r,"T"),1,"circle period frequency");
      near(value(r,"F_z"),mass*speed*speed/radius,"circle centripetal force");
    }
  }
  auto r=run("M_FLAT_CURVE",{50,0.5,9.81});near(value(r,"v_max"),std::sqrt(50*0.5*9.81),"flat curve bound");
  r=run("M_BANKED_CURVE",{50,pi/6,0,9.81});
  near(value(r,"v_min"),std::sqrt(50*9.81/std::sqrt(3.0)),"frictionless bank speed");near(value(r,"v_max"),value(r,"v_min"),"unique frictionless bank speed");
  r=run("M_BANKED_CURVE",{50,0,0.5,9.81});near(value(r,"v_min"),0,"zero-bank minimum");near(value(r,"v_max"),std::sqrt(50*9.81*0.5),"zero bank matches flat curve");
  r=run("M_BANKED_CURVE",{50,pi/3,1,9.81});near(value(r,"Endliche_Obergrenze"),0,"unbounded ideal bank limit represented without infinity");check(!has(r,"v_max"),"no infinite bank speed");
  r=run("M_CAROUSEL",{0,2,pi/6,60,9.81});near(value(r,"r"),1,"conical pendulum radius");
  near(value(r,"F_Seil")*std::cos(pi/6),60*9.81,"vertical tension balance");
  near(value(r,"F_Seil")*std::sin(pi/6),value(r,"F_z"),"horizontal tension balance");
  r=run("M_CAROUSEL",{3,2,pi/6,60,9.81});near(value(r,"r"),4,"carousel suspension offset");
  r=run("M_LOOP",{60,5,12.5,0,pi,9.81});near(value(r,"Voller_Loop"),1,"minimum looping energy");
  near(value(r,"N"),0,"contact at critical top");near(value(r,"v"),std::sqrt(5*9.81),"critical top speed");near(value(r,"h_min_Start"),12.5,"minimum looping height");
  r=run("M_LOOP",{60,5,4,0,pi,9.81});near(value(r,"Voller_Loop"),0,"insufficient looping height");near(value(r,"Kontakt_am_Ziel"),0,"unreachable top");
  near(value(r,"phi_Grenze"),std::acos(0.2),"loop turning point below side");check(!has(r,"v")&&!has(r,"N"),"no fictitious contact force beyond turning point");
  r=run("M_LOOP",{60,5,8,0,pi,9.81});near(value(r,"phi_Grenze"),std::acos(-0.4),"loop contact-loss angle");near(value(r,"Kontakt_am_Ziel"),0,"top after contact loss");
  r=run("M_LOOP",{60,5,8,0,pi/2,9.81});near(value(r,"Kontakt_am_Ziel"),1,"side reached before detachment");near(value(r,"N"),60*9.81*1.2,"side normal force");
  r=run("M_LOOP",{60,5,0,std::sqrt(6*9.81*5),2*pi,9.81});near(value(r,"Voller_Loop"),1,"loop supplied by start speed");near(value(r,"h_min_Start"),0,"start speed eliminates required height");
  reject("M_CIRCLE_V",{0,1,1});reject("M_CIRCLE_V",{1,0,1});reject("M_CIRCLE_T",{1,-1,1});
  reject("M_FLAT_CURVE",{10,-1,9.81});reject("M_BANKED_CURVE",{50,pi/2,0,9.81});
  reject("M_CAROUSEL",{0,2,0,60,9.81});reject("M_LOOP",{1,1,-1,0,pi,9.81});
  reject("M_LOOP",{1,1,1,0,7,9.81});reject("M_LOOP",{1,1e-200,0,1,pi,1e-200});
}

void unitsAndExtremes(){
  near(scale("km/h")*80,80*1000.0/3600,"km/h to SI");near(scale("deg")*30,pi/6,"degrees to radian SI");
  near(scale("g")*40,0.04,"grams to SI");
  auto r=run("M_ELASTIC",{40*scale("g"),60*scale("g"),4,0});near(value(r,"v1_nach"),-0.8,"gram unit fixture");
  r=run("M_OBLIQUE",{0,100*scale("km/h"),30*scale("deg"),9.81});near(value(r,"Reichweite"),68.117174131,"scaled speed and angle fixture");
  const double huge=std::numeric_limits<double>::max();
  reject("M_FREE",{huge,huge});reject("M_BRAKE_A",{huge,1,1,100});reject("M_VECTOR_SUM",{2,huge,0,huge,0});
  reject("M_WORK_ANGLE",{huge,huge,0,1});reject("M_ENERGY_V",{huge,huge,1,1,0,0,9.81});
  reject("M_ELASTIC",{huge,huge,huge,-huge});reject("M_CIRCLE_V",{1,huge,huge});
  reject("M_ENCOUNTER",{1,huge,huge,0,0,10,10,10,0});
  // The sum of these masses overflows, but their normalized momentum fractions do not.
  r=run("M_INELASTIC",{1e308,1e308,1e-200,1e-200});
  check(value(r,"v1_nach")>0,"mass normalization preserves tiny common velocity");
  near(value(r,"v1_nach")/1e-200,1,"normalized huge-mass collision");
}
}
int main(){
  catalogue();originalProblems();projectileTests();projectileHeightTests();brakingTests();encounterTests();vectorTests();
  energyTests();collisionTests();inclineTests();circularTests();unitsAndExtremes();
  if(failures){std::fprintf(stderr,"FAIL mechanics: %u of %u checks failed\n",failures,checks);return 1;}
  std::printf("PASS mechanics: %u checks; 8 categories; 38 workflows\n",checks);return 0;
}

// SPDX-License-Identifier: GPL-3.0-or-later
// Independent host executable: link only this file and src/fields.cpp.
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
constexpr double eps=8.8541878128e-12,mu=1.25663706212e-6;
unsigned checks=0;
std::map<std::string,Workflow> catalog;
std::set<std::string> numerical;
void need(bool ok,const std::string& message) {
  ++checks;
  if (!ok) { std::cerr<<"FAIL FIELDS: "<<message<<'\n'; std::exit(1); }
}
void equal(double actual,double expected,const std::string& context,double relative=2e-9,double absolute=1e-290) {
  need(std::isfinite(actual) && std::isfinite(expected) &&
       std::abs(actual-expected)<=std::max(absolute,relative*std::abs(expected)),context);
}
void clean(const NativeResult& r) {
  need(!r.values.empty(),"nonempty result");
  std::set<std::string> names;
  for (const auto& value:r.values) {
    need(!value.name.empty() && names.insert(value.name).second,"unique nonempty value names");
    need(std::isfinite(value.value),"finite value "+value.name);
  }
  for (const auto& p:r.plots) {
    need(!p.title.empty() && !p.xLabel.empty() && !p.yLabel.empty(),"plot labels");
    need(p.points.size()<=400 && p.model.size()<=400,"plot bound");
    for (const auto& point:p.points) need(std::isfinite(point.first)&&std::isfinite(point.second),"finite point");
    for (const auto& point:p.model) need(std::isfinite(point.first)&&std::isfinite(point.second),"finite model");
  }
}
NativeResult solve(const std::string& id,const std::vector<double>& values) {
  NativeResult result;
  std::string error="old error";
  need(fieldsCalculate(id,values,result,error),id+": "+error);
  need(error.empty(),id+": cleared error"); clean(result); return result;
}
double value(const NativeResult& r,const std::string& name,const std::string& unit) {
  for (const auto& v:r.values) if (v.name==name) { need(v.unit==unit,"unit "+name); return v.value; }
  need(false,"missing result "+name); return 0;
}
void expect(const std::string& id,const std::vector<double>& input,const std::string& name,double expected,const std::string& unit="",double relative=2e-9) {
  numerical.insert(id); equal(value(solve(id,input),name,unit),expected,id+": "+name,relative);
}
void reject(const std::string& id,const std::vector<double>& values) {
  NativeResult r;
  r.values.push_back({"stale",42,"V"}); r.note="stale";
  r.plots.push_back({"stale","x","s","y","m",{{1,2}},{},true});
  std::string error;
  need(!fieldsCalculate(id,values,r,error),id+": expected rejection");
  need(!error.empty(),id+": German error present");
  need(r.values.empty() && r.plots.empty() && r.note.empty(),id+": cleared failed result");
}
double initial(const std::string& text,const Parameter& parameter) {
  char* end=nullptr;
  const double n=std::strtod(text.c_str(),&end);
  need(end && *end=='\0' && std::isfinite(n),"valid default number");
  if (parameter.unit=="rad") { need(parameter.defaultUnit=="deg","degrees UI / SI radians"); return n*pi/180; }
  return n;
}
std::vector<double> defaults(const Workflow& w) {
  std::vector<double> v;
  for (size_t i=0;i<w.defaults.size();++i) v.push_back(initial(w.defaults[i],w.calculation.parameters[i]));
  if (w.maxCount) for (unsigned i=0;i<static_cast<unsigned>(v[0]);++i)
    for (size_t j=0;j<w.repeatedDefaults.size();++j) v.push_back(initial(w.repeatedDefaults[j],w.repeated[j]));
  return v;
}

void contract() {
  const auto categories=fieldsCategories();
  need(categories.size()==8,"8 categories");
  std::set<std::string> categoryIds;
  for (const auto& cat:categories) {
    need(!cat.id.empty() && categoryIds.insert(cat.id).second && !cat.title.empty(),"category identity");
    const auto workflows=fieldsCatalog(cat.id);
    need(!workflows.empty(),"category not empty");
    std::cout<<cat.id<<": "<<workflows.size()<<" workflows\n";
    for (const auto& w:workflows) {
      const auto& calc=w.calculation;
      need(calc.id.rfind("E_",0)==0 && catalog.emplace(calc.id,w).second,"unique stable E_ id");
      need(!calc.title.empty() && !calc.formula.empty() && !calc.name.empty(),"workflow text");
      need(calc.parameters.size()==w.defaults.size(),"base defaults shape");
      need(w.repeated.size()==w.repeatedDefaults.size(),"repeated defaults shape");
      need(w.minCount<=w.maxCount && w.maxCount<=40,"count bounds");
      need(w.maxCount==0 ? w.repeated.empty() : !w.repeated.empty(),"dynamic shape");
      for (const auto& p:calc.parameters) need(!p.name.empty(),"parameter name");
      const auto input=defaults(w);
      solve(calc.id,input);
      reject(calc.id,{});
      auto wrong=input; wrong.pop_back(); reject(calc.id,wrong);
      wrong=input; wrong.push_back(1); reject(calc.id,wrong);
      for (size_t i=0;i<input.size();++i) for (double x:{std::numeric_limits<double>::quiet_NaN(),
          std::numeric_limits<double>::infinity(),-std::numeric_limits<double>::infinity()}) {
        wrong=input; wrong[i]=x; reject(calc.id,wrong);
      }
      if (w.maxCount) {
        for (double n:{0.0,1.5,9.0,1e300,-1.0}) { wrong=input; wrong[0]=n; reject(calc.id,wrong); }
        for (unsigned n:{w.minCount,w.maxCount}) {
          wrong.assign(input.begin(),input.begin()+static_cast<std::ptrdiff_t>(calc.parameters.size()));
          wrong[0]=n;
          for (unsigned j=0;j<n;++j) for (size_t k=0;k<w.repeated.size();++k)
            wrong.push_back(initial(w.repeatedDefaults[k],w.repeated[k]));
          solve(calc.id,wrong);
        }
      }
    }
  }
  need(catalog.size()==112,"112 workflows");
  need(fieldsCatalog("unknown").empty(),"unknown category");
  reject("E_UNKNOWN",{1,2,3}); reject("",{});
}

void electric() {
  // Definition E=F/q: synthetic signed test-charge and inverse checks.
  expect("E_FIELD_E",{0.006,2e-6},"E",3000,"V/m");
  expect("E_FIELD_F",{3000,-2e-6},"F",-0.006,"N");
  expect("E_FIELD_Q",{-0.006,3000},"q",-2e-6,"C");
  const double force=-1e-12/(4*pi*eps*0.1*0.1);
  expect("E_COULOMB_F",{1e-6,-1e-6,0.1,1},"F radial",force,"N");
  need(solve("E_COULOMB_F",{1e-6,-1e-6,0.1,1}).note.find("Anziehung")!=std::string::npos,"signed attraction");
  expect("E_COULOMB_R",{1e-6,-1e-6,force,1},"r",0.1,"m");
  expect("E_COULOMB_Q",{force,-1e-6,0.1,1},"q1",1e-6,"C");
  expect("E_COULOMB_ER",{1e-6,-1e-6,force/3.9,0.1},"er",3.9);
  const double field=70e-9/(4*pi*eps*3.9*0.07*0.07);
  expect("E_POINT_E",{70e-9,0.07,3.9},"E radial",field,"V/m"); // Mixed 3.12.
  expect("E_POINT_Q",{field,0.07,3.9},"Q",70e-9,"C");
  expect("E_POINT_R",{70e-9,field,3.9},"r",0.07,"m");
  // Mixed 2.1: force on Q1 at (1,0,5), due to Q2 at (9,9,0).
  const auto original=solve("E_SUPERPOSITION",{1,1,1,0,5,-50e-6,60e-6,9,9,0});
  const double dist=std::sqrt(170.0),k=1/(4*pi*eps);
  equal(value(original,"Fx","N"),k*(-50e-6)*60e-6*(-8)/(170*dist),"mixed 2.1 Fx");
  equal(value(original,"Fy","N"),k*(-50e-6)*60e-6*(-9)/(170*dist),"mixed 2.1 Fy");
  equal(value(original,"Fz","N"),k*(-50e-6)*60e-6*5/(170*dist),"mixed 2.1 Fz");
  expect("E_SUPERPOSITION",{2,1,0,0,0,0,1e-6,-1,0,0,1e-6,1,0,0},"E",0,"V/m");
  expect("E_SUPERPOSITION",{2,1,0,0,0,-2e-9,1e-6,-1,0,0,-1e-6,1,0,0},"Fx",-2e-9*2*k*1e-6,"N");
  // Unit-scale: dm coordinates and uC charges converted once to SI.
  const auto dm=solve("E_SUPERPOSITION",{1,1,0.1,0.2,0.3,-20e-6,40e-6,-0.3,0.1,-0.5});
  const auto si=solve("E_SUPERPOSITION",{1,1,1*0.1,2*0.1,3*0.1,-20*1e-6,40*1e-6,-3*0.1,1*0.1,-5*0.1});
  equal(value(dm,"F","N"),value(si,"F","N"),"mixed 2.4 dm scaling");
  const auto farther=solve("E_SUPERPOSITION",{1,1,1,2,3,-20e-6,40e-6,-3,1,-5});
  equal(value(farther,"F","N"),value(dm,"F","N")/100,"3D inverse-square length scaling");

  // Original mixed 3.3 and IQB Defibrillator 1-4.
  expect("E_CAP_STATE",{1.5e-6,220},"Q",330e-6,"C");
  expect("E_CAP_STATE",{1.5e-6,220},"W",0.0363,"J");
  const auto defib=solve("E_CAP_STATE",{50e-6,4000});
  equal(value(defib,"Q","C"),0.2,"defib Q"); equal(value(defib,"W","J"),400,"defib W");
  expect("E_CAP_U_ENERGY",{50e-6,300},"U Betrag",3464.1016151377544,"V");
  const double capacity=eps*2.1*0.02/0.008;
  expect("E_CAP_GEOMETRY",{0.02,0.008,2.1,400},"C",capacity,"F");
  expect("E_CAP_AREA",{capacity,0.008,2.1},"A",0.02,"m^2");
  expect("E_CAP_DISTANCE",{capacity,0.02,2.1},"d",0.008,"m");
  expect("E_CAP_ER",{capacity,0.02,0.008},"er",2.1);
  // Original mixed 3.19, 3.20: isolated vs connected, geometry/dielectric changes.
  expect("E_CAP_ISOLATED",{0.02,0.008,1,400,0.02,0.02,1},"U",1000,"V");
  expect("E_CAP_CONNECTED",{0.02,0.008,1,400,0.02,0.02,1},"U",400,"V");
  expect("E_CAP_ISOLATED",{0.02,0.008,1,220,0.02,0.008,2.1},"U",220/2.1,"V");
  // Original mixed 3.10: 1.5/3.5 uF series at 110 V.
  expect("E_CAP_SERIES",{2,110,1.5e-6,3.5e-6},"C",1.05e-6,"F");
  expect("E_CAP_SERIES",{2,110,1.5e-6,3.5e-6},"U1",77,"V");
  expect("E_CAP_SERIES",{2,110,1.5e-6,3.5e-6},"U2",33,"V");
  expect("E_CAP_SERIES",{2,110,1.5e-6,3.5e-6},"Q2",115.5e-6,"C");
  expect("E_CAP_PARALLEL",{2,110,1.5e-6,3.5e-6},"C",5e-6,"F");
  expect("E_CAP_PARALLEL",{2,-110,1.5e-6,3.5e-6},"Q2",-385e-6,"C");
  expect("E_RC_CONSTANTS",{50e-6,100},"tau",0.005,"s");
  expect("E_RC_CONSTANTS",{50e-6,100},"tHalfU",0.0034657359027997266,"s");
  expect("E_RC_CONSTANTS",{50e-6,100},"tHalfW",0.0017328679513998633,"s");
  expect("E_RC_STATE",{50e-6,100,4000,0,0},"Q",0.2,"C");
  expect("E_RC_STATE",{50e-6,100,4000,0,0.005},"U",4000/std::exp(1.0),"V");
  expect("E_RC_STATE",{50e-6,100,4000,0,0.005},"I",-40/std::exp(1.0),"A");
  const auto discharge=solve("E_RC_STATE",{50e-6,100,4000,0,0.006});
  equal(value(discharge,"W","J")+value(discharge,"lostEnergy","J"),400,"RC energy balance");
  expect("E_RC_TIME_U",{50e-6,100,4000,0,2000},"t",0.005*std::log(2.0),"s");
  expect("E_RC_TIME_ENERGY",{50e-6,100,-4000,200},"t",0.0025*std::log(2.0),"s");
  // General source / nonzero initial voltage, crossing zero, and negative-source inverse.
  expect("E_RC_STATE",{1,2,-3,5,2*std::log(2.0)},"U",1,"V");
  expect("E_RC_TIME_U",{1,2,-3,5,1},"t",2*std::log(2.0),"s");
  expect("E_RC_TIME_U",{1,2,5,-3,1},"t",2*std::log(2.0),"s");
  expect("E_RC_STATE",{1,2,5,-3,2*std::log(2.0)},"lostEnergy",24,"J");
  expect("E_RC_STATE",{1e-6,1e3,0,12,0},"lostEnergy",0,"J");
  expect("E_RC_TIME_U",{1,2,3,5,3},"t",0,"s");
  expect("E_RC_TIME_Q",{50e-6,100,4000,0,0.1},"t",0.005*std::log(2.0),"s");
  expect("E_RC_TIME_I",{50e-6,100,4000,0,-20},"t",0.005*std::log(2.0),"s");
  expect("E_RC_TIME_W_POS",{1,2,-3,5,0.5},"t",2*std::log(2.0),"s");
  expect("E_RC_TIME_W_NEG",{1,2,-3,5,0.5},"t",-2*std::log(0.75),"s");
  expect("E_RC_TIME_W_POS",{1,2,-3,5,0},"t",-2*std::log(5.0/8),"s");
  expect("E_RC_TIME_LOSS",{50e-6,100,4000,0,200},"t",0.0025*std::log(2.0),"s");
  expect("E_RC_TIME_LOSS",{1,1,1,0,0},"t",0,"s");
}

void magnetic() {
  expect("E_WIRE_F",{820*1e-3,18,128*1e-3,pi/2},"F",1.88928,"N"); // Mixed 5.12.
  expect("E_WIRE_B",{1.88928,18,0.128,pi/2},"B",0.82,"T");
  expect("E_WIRE_I",{1.88928,0.82,0.128,pi/2},"I Betrag",18,"A");
  expect("E_WIRE_LENGTH",{1.65,0.34,14.5,38*pi/180},"l",1.65/(0.34*14.5*std::sin(38*pi/180)),"m"); // Mixed 5.13.
  expect("E_WIRE_ANGLE",{0.5,2,1,0.5},"alpha",pi/6,"rad");
  const double lorentz=charge*1e7*0.001*0.5;
  expect("E_CHARGE_F",{-charge,1e7,0.001,pi/6},"F",lorentz,"N");
  expect("E_CHARGE_B",{lorentz,-charge,1e7,pi/6},"B",0.001,"T");
  expect("E_CHARGE_Q",{lorentz,1e7,0.001,pi/6},"q Betrag",charge,"C");
  expect("E_CHARGE_V",{lorentz,-charge,0.001,pi/6},"v",1e7,"m/s");
  expect("E_CHARGE_ANGLE",{lorentz,-charge,1e7,0.001},"alpha",pi/6,"rad");
  expect("E_WIRE_VECTOR",{-2,0.1,0,0,0,0,3},"Fy",0.6,"N");
  expect("E_CHARGE_VECTOR",{-charge,1e7,0,0,0,0,0.001},"Fy",charge*1e4,"N");
  // Fully 3D cross product (1,2,3) x (4,5,6) = (-3,6,-3).
  expect("E_WIRE_VECTOR",{2,1,2,3,4,5,6},"Fx",-6,"N");
  expect("E_WIRE_VECTOR",{2,1,2,3,4,5,6},"Fy",12,"N");
  expect("E_WIRE_VECTOR",{2,1,2,3,4,5,6},"Fz",-6,"N");
  const double bfield=mu*820/0.64;
  expect("E_SOLENOID_B",{1,820,1,0.64},"B",bfield,"T");
  expect("E_SOLENOID_I",{bfield,1,820,0.64},"I",1,"A");
  expect("E_SOLENOID_N",{bfield,1,1,0.64},"N",820);
  expect("E_SOLENOID_MU",{bfield,820,1,0.64},"mur",1);
  expect("E_SOLENOID_LENGTH",{bfield,1,820,1},"l",0.64,"m");
  expect("E_LONGWIRE_B",{1,2,0.1},"B",mu/(0.1*pi),"T");
  expect("E_LONGWIRE_I",{mu/(0.1*pi),1,0.1},"I",2,"A");
  expect("E_LONGWIRE_R",{mu/(0.1*pi),1,2},"r",0.1,"m");
  expect("E_PARALLEL_F",{1,1,1,1,1},"F radial",-mu/(2*pi),"N"); // Mixed 5.3.
  expect("E_PARALLEL_F",{1,-1,1,1,1},"F radial",mu/(2*pi),"N");
  expect("E_PARALLEL_I",{-0.0034,58,0.12,2.8,1},"I1",0.0034*2*pi*0.12/(mu*58*2.8),"A"); // Mixed 5.11.
  const double wireArea=pi*0.0005*0.0005/4,resistance=1.78e-8*10/wireArea;
  expect("E_WIRE_RESISTANCE",{1.78e-8,10,0.0005},"R",resistance,"Ohm");
  expect("E_WIRE_RESISTANCE",{0.0178*1e-6,10,0.5*1e-3},"R",resistance,"Ohm"); // Ohm mm^2/m and mm -> SI.
  expect("E_WIRE_LENGTH_R",{resistance,1.78e-8,0.0005},"Drahtlaenge",10,"m");
  // Original mixed 5.18: 65 mm / 6 mm / N=100 / wire 0.5 mm / 10 V.
  const double lw=pi*0.006*100,rr=1.78e-8*lw/wireArea,ii=10/rr;
  const double inductance=mu*100*100*(pi*0.006*0.006/4)/0.065;
  expect("E_COIL_WIRE",{100,0.065,0.006,0.0005,1.78e-8,1,10},"R",rr,"Ohm");
  expect("E_COIL_WIRE",{100,0.065,0.006,0.0005,1.78e-8,1,10},"W",0.5*inductance*ii*ii,"J");
  // Original mixed 5.21: 0.3 m core, diameter 0.05 m, wire 0.8 mm, mur=880.
  const double turns=375,denseWire=pi*(0.05+0.0008)*turns;
  const double denseR=1.78e-8*denseWire/(pi*0.0008*0.0008/4);
  expect("E_WINDING",{0.3,0.05,0.0008,1,1.78e-8,880,20},"N",375);
  expect("E_WINDING",{0.3,0.05,0.0008,1,1.78e-8,880,20},"I",20/denseR,"A");
  expect("E_WINDING",{0.3,0.05,0.0008,1,1.78e-8,880,20},"L",mu*880*turns*turns*(pi*0.05*0.05/4)/0.3,"H");
  expect("E_WINDING_H",{0.0002,2,0.52,1},"H",5200,"A/m"); // Mixed 5.15.
  const double path=pi*0.1,cross=pi*0.01*0.01;
  expect("E_RING",{450,0.04,0.06,1,1},"L",mu*450*450*cross/path,"H");
  expect("E_RING_CURRENT",{450,0.04,0.06,1,2e-6},"I",2e-6*path/(mu*450*cross),"A"); // Mixed 5.4.
  // Original mixed 5.20: ri=200 mm, ro=250 mm, wire 1.2 mm, I=2.5 A, mur=400.
  const double rn=1047,rm=pi*0.45,ra=pi*0.025*0.025;
  const double rb=mu*400*rn*2.5/rm;
  expect("E_RING_WINDING_I",{0.2,0.25,0.0012,2.5,400},"N",rn);
  expect("E_RING_WINDING_I",{0.2,0.25,0.0012,2.5,400},"B",rb,"T");
  expect("E_RING_WINDING_I",{0.2,0.25,0.0012,2.5,400},"Phi",rb*ra,"Wb");
  const double ringR=1.78e-8*pi*(0.05+0.0012)*rn/(pi*0.0012*0.0012/4);
  expect("E_RING_WINDING",{0.2,0.25,0.0012,1.78e-8,400,2.5*ringR,0.002},"I",2.5,"A");
  // Original mixed 5.22: da=2.5 m, di=1.2 m; Konstantan rho is editable.
  const double diameter=std::sqrt(4e-5/pi),rn2=1056,ringWire2=pi*(0.65+diameter)*rn2;
  const double ringL2=mu*250*rn2*rn2*pi*0.325*0.325/(pi*1.85),ringI2=310/(4.9e-7*ringWire2/1e-5);
  expect("E_RING_WINDING_AREA",{0.6,1.25,1e-5,4.9e-7,250,310,0.002},"N",rn2);
  expect("E_RING_WINDING_AREA",{0.6,1.25,1e-5,4.9e-7,250,310,0.002},"L",ringL2,"H");
  expect("E_RING_WINDING_AREA",{0.6,1.25,1e-5,4.9e-7,250,310,0.002},"Uind mittel",ringL2*ringI2/0.002,"V");
  const double n=std::sqrt(0.05*0.06/(mu*2.827433388e-5));
  expect("E_COIL_N_L",{0.05,2.827433388e-5,1,0.06},"N",n); // Mixed 5.6.
  expect("E_COIL_N_H",{1.5,0.0005,1900,500,0.3},"N",1.5*0.3/(mu*1900*0.0005*500)); // Mixed 5.10.
  expect("E_COIL_LINKAGE",{0.06,5,0.015},"N",20); // Mixed 5.16.
}

void particles() {
  // Komplexaufgabe Fadenstrahlrohr, actual measured U=200 V, B=1.25 mT, r=3.9 cm.
  expect("E_PARTICLE_QM",{200,1.25e-3,3.9e-2},"q/m",400/std::pow(0.00125*0.039,2),"C/kg");
  const double q=charge,m=electronMass,b=0.00125,v=std::sqrt(2*q*200/m),radius=m*v/(q*b),period=2*pi*m/(q*b);
  expect("E_PARTICLE_ORBIT",{200,-q,m,b},"v",v,"m/s");
  expect("E_PARTICLE_ORBIT",{200,-q,m,b},"r",radius,"m");
  expect("E_PARTICLE_B",{200,-q,m,radius},"B",b,"T");
  expect("E_PARTICLE_U",{radius,b,-q,m},"Uacc",200,"V");
  expect("E_PARTICLE_SPEED",{v,-q,m,b},"T",period,"s");
  expect("E_PARTICLE_SPEED",{v,-q,m,b},"f",1/period,"Hz");
  expect("E_PARTICLE_V",{200,-q,m},"v",v,"m/s");
  expect("E_PARTICLE_U_V",{v,-q,m},"Uacc",200,"V");
  expect("E_PARTICLE_M",{200,-q,b,radius},"m",m,"kg");
  expect("E_PARTICLE_Q",{200,m,b,radius},"q Betrag",q,"C");
  expect("E_PARTICLE_B_T",{period,-q,m},"B",b,"T");
  expect("E_PARTICLE_B_F",{1/period,-q,m},"B",b,"T");
  expect("E_PARTICLE_QM_T",{b,period},"q/m",q/m,"C/kg");
  expect("E_PARTICLE_V_R",{radius,b,-q,m},"v",v,"m/s");
  // Original mixed 4.1: 1e7 m/s, 60 V, 1 cm gap, 3 cm plates, screen 20 cm after plates.
  const double accel=-q*60/(m*0.01),t=0.03/1e7,y=0.5*accel*t*t,vy=accel*t;
  expect("E_TRANSVERSE",{-q,m,1e7,60,0.01,0.03,0.2},"y Schirm",y+vy*0.2/1e7,"m");
  expect("E_TRANSVERSE",{-q,m,1e7,60,0.01,0.03,0.2},"Plattentreffer",0);
  expect("E_TRANSVERSE",{-q,m,1e7,60,0.01,0.03,0.2},"Fy",-q*6000,"N");
  const double accU=m*1e14/(2*q);
  expect("E_TRANSVERSE_ACC",{-q,m,accU,60,0.01,0.03,0.2},"y Schirm",y+vy*0.2/1e7,"m");
  const auto hit=solve("E_TRANSVERSE",{-q,m,1e6,60,0.01,0.03,0.2});
  equal(value(hit,"Plattentreffer",""),1,"plate hit detected");
  equal(std::abs(value(hit,"y Feld","m")),0.005,"plate boundary");
  for (const auto& item:hit.values) need(item.name!="y Schirm" && item.name!="t Schirm","no imaginary screen after plate hit");
  need(hit.plots[0].points.back().first<0.03,"plot stops at plate hit");
  expect("E_TRANSVERSE",{q,m,1e7,0,0.01,0.03,0.2},"y Schirm",0,"m");
  expect("E_WIEN_V",{1500,0.0015},"v",1e6,"m/s");
  expect("E_WIEN_E",{1e6,0.0015},"E",1500,"V/m");
  expect("E_WIEN_B",{1500,1e6},"B",0.0015,"T");
  const double ionMass=6.64e-27,ionQ=2*q,ionR=ionMass*1e6/(ionQ*0.1);
  expect("E_MASSSPEC_R",{ionMass,ionQ,60,0.04,0.0015,0.1},"r",ionR,"m");
  expect("E_MASSSPEC",{60,0.04,0.0015,0.1,ionR,ionQ},"m",ionMass,"kg");
  const double proton=1.67262192369e-27,energy=200*q*1000,cv=std::sqrt(2*energy/proton),cr=proton*cv/q;
  expect("E_CYCLOTRON",{q,proton,1,1000,100},"W",energy,"J");
  expect("E_CYCLOTRON",{q,proton,1,1000,100},"Spaltpassagen",200);
  expect("E_CYCLOTRON_TURNS",{q,proton,1,1000,cr},"Umlaeufe",100);
  // IQB Millikan 2.1. Very small values use relative comparison, not a 1e-10 absolute floor.
  const double mass=4*pi*1e-18/3*2300,mq=mass*9.81*0.001/115;
  expect("E_MILLIKAN_RADIUS",{1e-6,2300,0,0.001,115,9.81},"m",9.634217471008698e-15,"kg");
  expect("E_MILLIKAN_RADIUS",{1e-6,2300,0,0.001,115,9.81},"q",mq,"C");
  expect("E_MILLIKAN_RADIUS",{1e-5*0.1,2.3*1e3,0,1*1e-3,115,9.81},"q",mq,"C"); // Original dm, kg/dm^3, mm.
  expect("E_MILLIKAN_MASS",{mass,2300,0,0.001,-115,9.81},"q",-mq,"C");
  expect("E_MILLIKAN_MASS",{mass,2300,1.2,0.001,115,9.81},"q",mq*(1-1.2/2300),"C");
  // Preserve the old module untouched; this module warns, rather than truncates at 0.1c.
  need(solve("E_PARTICLE_U_V",{0.1*light,-q,m}).note.find("Warnung")!=std::string::npos,"warning at exactly 0.1c");
  need(solve("E_PARTICLE_U_V",{0.09*light,-q,m}).note.find("Warnung")==std::string::npos,"below warning threshold");
}

void inductionAndCircuits() {
  expect("E_FLUX",{0.1,0.01,pi/3},"Phi",0.0005,"Wb");
  expect("E_FLUX",{0.1,0.01,pi},"Phi",-0.001,"Wb");
  expect("E_FLUX",{0.1,0.01,pi/2},"Phi",0,"Wb");
  expect("E_FLUX_B",{0.0005,0.01,pi/3},"B",0.1,"T");
  expect("E_FLUX_A",{-0.001,0.1,pi},"A",0.01,"m^2");
  expect("E_INDUCTION_MEAN",{100,0,0.001,0.1},"Uind mittel",-1,"V");
  expect("E_INDUCTION_STATES",{100,0.1,0.01,0,0.1,0.01,pi/2,0.1},"Uind mittel",1,"V");
  expect("E_MOVING_CONDUCTOR",{0.5,0,0,0,0,0.04,0,-1.5,0},"U",0.03,"V");
  // 3D dot product, including reversal of the oriented conductor.
  expect("E_MOVING_CONDUCTOR",{1,2,3,4,5,6,2,3,4},"U",0,"V");
  expect("E_MOVING_CONDUCTOR",{1,2,3,4,5,6,1,2,3},"U",0,"V");
  expect("E_MOVING_CONDUCTOR",{1,2,3,4,5,6,1,0,0},"U",-3,"V");
  expect("E_MOVING_CONDUCTOR",{1,2,3,4,5,6,-1,0,0},"U",3,"V");
  expect("E_SELF",{0.9,0,0.04,0.003},"Uind mittel",-12,"V");
  expect("E_SELF_L",{-12,0,0.04,0.003},"L",0.9,"H");
  const double ca=pi*std::pow(0.06/(2*pi),2),cl=mu*850*850*ca/0.15;
  expect("E_COIL_L",{1,850,ca,0.15},"L",cl,"H"); // Original mixed 5.5 geometry.
  // Original induction worksheet RL data: L=0.9 H, R=330 Ohm, U=12 V.
  const double tau=0.9/330,steady=12.0/330;
  expect("E_RL_ON",{0.9,330,12,0,tau},"tau",tau,"s");
  expect("E_RL_ON",{0.9,330,12,0,tau},"I",steady*(1-std::exp(-1.0)),"A");
  expect("E_RL_ON",{0.9,330,12,0,0},"UL",12,"V");
  expect("E_RL_ON",{0.9,330,12,0,0},"Uind",-12,"V");
  expect("E_RL_TIME_ON",{0.9,330,12,0,steady/2},"t",tau*std::log(2.0),"s");
  expect("E_RL_TIME_OFF",{0.9,100,steady,steady/2},"t",0.009*std::log(2.0),"s");
  expect("E_RL_OFF",{0.9,330,100,12,0.009},"I",steady/std::exp(1.0),"A");
  expect("E_RL_OFF",{0.9,330,100,12,0},"UL",-100*steady,"V");
  expect("E_RL_OFF_INITIAL",{0.9,100,steady,0.009},"I",steady/std::exp(1.0),"A");
  const auto off=solve("E_RL_OFF_INITIAL",{0.9,100,steady,0.009});
  equal(value(off,"W","J")+value(off,"lostEnergy","J"),0.5*0.9*steady*steady,"RL off energy balance");
  // Induction transformer worksheet: 230/8 V, N1=5500, I2=2.6 A.
  const double n2=5500*8.0/230,i1=8*2.6/230;
  expect("E_TRAFO_N2",{230,8,5500},"N2",n2);
  expect("E_TRAFO_N1",{230,8,n2},"N1",5500);
  expect("E_TRAFO_U1",{8,5500,n2},"U1",230,"V");
  expect("E_TRAFO_IDLE",{230,5500,n2},"U2",8,"V");
  expect("E_TRAFO_IDLE",{230,5500,n2},"I1",0,"A");
  expect("E_TRAFO_LOAD",{230,5500,n2,2.6},"I1",i1,"A");
  expect("E_TRAFO_FROM_V",{230,8,5500,2.6,1},"I1",i1,"A");
  expect("E_TRAFO_ETA",{230,5500,n2,2.6,0.8},"U2",8,"V");
  expect("E_TRAFO_ETA",{230,5500,n2,2.6,0.8},"I1",i1/0.8,"A");
  expect("E_TRAFO_EFFICIENCY",{230,i1/0.8,8,2.6},"eta",0.8);
  expect("E_TRAFO_I2",{230,5500,n2,i1/0.8,0.8},"I2",2.6,"A");
}

void invalidAndExtremes() {
  const auto rcStart=solve("E_RC_STATE",{1,1,1,1e20,0});
  equal(value(rcStart,"U","V"),1,"RC preserves t=0 with large source");
  equal(rcStart.plots.front().points.front().second,1,"RC graph preserves initial condition");
  equal(value(solve("E_RC_STATE",{1,1,1,1e20,1e-20}),"U","V"),2,"RC tiny elapsed time retains source increment");
  const auto rlStart=solve("E_RL_ON",{1,1,1e20,1,0});
  equal(value(rlStart,"I","A"),1,"RL preserves t=0 with large source");
  equal(rlStart.plots.front().points.front().second,1,"RL graph preserves initial condition");
  equal(value(solve("E_POINT_E",{1e308,1e308,1}),"E radial","V/m"),1/(4*pi*eps)*1e-308,"point field avoids denominator overflow",2e-9,0);
  equal(value(solve("E_COULOMB_F",{1e308,1e308,1e308,1}),"F radial","N"),1/(4*pi*eps),"Coulomb balanced large factors");
  reject("E_MOVING_CONDUCTOR",{light,0,0,0,0,1,0,1,0});
  reject("E_FIELD_E",{1,0}); reject("E_FIELD_Q",{1,0});
  reject("E_COULOMB_F",{1,1,0,1}); reject("E_COULOMB_R",{1,-1,1,1});
  reject("E_COULOMB_ER",{1,-1,1,1}); reject("E_POINT_R",{-1,1,1});
  reject("E_POINT_E",{1,1,-1}); reject("E_SUPERPOSITION",{1,1,0,0,0,0,1,0,0,0});
  reject("E_CAP_STATE",{-1,220}); reject("E_CAP_U_ENERGY",{1,-1});
  reject("E_CAP_SERIES",{1,110,1e-6}); reject("E_CAP_SERIES",{2,110,1e-6,0});
  reject("E_CAP_PARALLEL",{2,110,-1e-6,1e-6}); reject("E_CAP_CONNECTED",{1,1,1,1,1,0,1});
  reject("E_RC_STATE",{1,0,1,0,1}); reject("E_RC_STATE",{1,1,1,0,-1});
  reject("E_RC_TIME_U",{1,1,1,0,0}); reject("E_RC_TIME_U",{1,1,1,1,1});
  reject("E_RC_TIME_U",{1,1,1,0,-1}); reject("E_RC_TIME_U",{1,1,1,0,2});
  reject("E_RC_TIME_ENERGY",{1,1,1,0}); reject("E_RC_TIME_ENERGY",{1,1,1,1});
  reject("E_RC_TIME_I",{1,1,1,0,1}); reject("E_RC_TIME_Q",{1,1,1,0,0});
  reject("E_RC_TIME_W_NEG",{1,1,1,2,1}); reject("E_RC_TIME_W_POS",{1,1,1,0,-1});
  reject("E_RC_TIME_LOSS",{1,1,1,0,0.5}); reject("E_RC_TIME_LOSS",{1,1,1,1,0});
  reject("E_CAP_SERIES",{2,1,std::numeric_limits<double>::denorm_min(),1e-6});
  reject("E_WIRE_B",{0,1,1,0}); reject("E_WIRE_I",{0,1,1,pi});
  reject("E_CHARGE_B",{0,1,1,0}); reject("E_WIRE_ANGLE",{2,1,1,1});
  reject("E_WIRE_F",{1,1,1,-0.1}); reject("E_WIRE_F",{1,1,1,pi+0.1});
  reject("E_SOLENOID_N",{-1,1,1,1}); reject("E_SOLENOID_MU",{1,1,-1,1});
  reject("E_LONGWIRE_R",{-1,1,1}); reject("E_PARALLEL_I",{1,0,1,1,1});
  reject("E_WIRE_RESISTANCE",{0,1,1}); reject("E_WINDING",{1,1,0.1,1.5,1,1,1});
  reject("E_WINDING",{1,1,0.1,41,1,1,1}); reject("E_WINDING",{0.1,1,1,1,1,1,1});
  reject("E_WINDING_H",{1,1.5,1,1}); reject("E_RING",{1,1,0.5,1,1});
  reject("E_RING_WINDING_I",{0.1,0.2,1,1,1}); reject("E_RING_WINDING_AREA",{1,1,1,1,1,1,1});
  reject("E_COIL_N_H",{1,1,1,-1,1}); reject("E_COIL_LINKAGE",{1,1,-1});
  reject("E_PARTICLE_ORBIT",{1,0,1,1}); reject("E_PARTICLE_ORBIT",{1,1,-1,1});
  reject("E_PARTICLE_U_V",{light,charge,electronMass}); reject("E_CHARGE_F",{charge,light,1,1});
  reject("E_CHARGE_VECTOR",{charge,light,light,0,1,1,1});
  reject("E_WIEN_V",{light,1}); reject("E_TRANSVERSE",{charge,electronMass,light,0,1,1,1});
  reject("E_CYCLOTRON",{charge,1,1,1,1.5}); reject("E_MILLIKAN_RADIUS",{1,1,2,1,1,1});
  reject("E_MILLIKAN_MASS",{1,1,1,1,1,1}); reject("E_MILLIKAN_MASS",{1,1,0,1,0,1});
  reject("E_FLUX_B",{0,1,pi/2}); reject("E_FLUX_A",{1,1,pi});
  reject("E_INDUCTION_MEAN",{1,0,1,0}); reject("E_SELF_L",{1,0,1,1});
  reject("E_SELF_L",{1,1,1,1}); reject("E_COIL_L",{1,1,1,0});
  reject("E_RL_ON",{1,0,1,0,1}); reject("E_RL_OFF",{1,1,0,1,1});
  reject("E_RL_TIME_OFF",{1,1,1,0}); reject("E_RL_TIME_ON",{1,1,1,1,1});
  reject("E_RL_TIME_ON",{1,1,1,0,1}); reject("E_RL_TIME_ON",{1,1,1,0,-1});
  reject("E_TRAFO_ETA",{230,5500,200,2.6,0}); reject("E_TRAFO_ETA",{230,5500,200,2.6,1.1});
  reject("E_TRAFO_EFFICIENCY",{1,1,2,2}); reject("E_TRAFO_N2",{0,8,5500});
  reject("E_FIELD_E",{1e308,1e-308}); reject("E_RC_STATE",{1e308,1e308,1,0,1});
  // Finite extreme inputs must either produce a completely finite result or an empty failure.
  for (const auto& entry:catalog) {
    const auto& id=entry.first; const auto original=defaults(entry.second);
    for (size_t i=0;i<original.size();++i) for (double x:{0.0,-1.0,1e-300,1e300}) {
      auto input=original; input[i]=x;
      NativeResult r; r.note="old"; r.values.push_back({"old",1,""}); std::string error="old";
      if (fieldsCalculate(id,input,r,error)) { need(error.empty(),"no error on finite success"); clean(r); }
      else need(!error.empty() && r.values.empty() && r.note.empty() && r.plots.empty(),"extreme failure clears everything");
    }
  }
}
}

int main() {
  contract(); electric(); magnetic(); particles(); inductionAndCircuits(); invalidAndExtremes();
  for (const auto& entry:catalog) need(numerical.count(entry.first)==1,"independent numerical coverage: "+entry.first);
  std::cout<<"PASS FIELDS: "<<checks<<" checks, "<<catalog.size()<<" workflows, 8 categories; original/inverse/SI/domain tests\n";
}

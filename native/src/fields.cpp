// SPDX-License-Identifier: GPL-3.0-or-later
#include "extensions.hpp"
#include <algorithm>
#include <cmath>
#include <initializer_list>
#include <limits>

namespace physik {
namespace {
constexpr double eps0=8.8541878128e-12, mu0=1.25663706212e-6;
enum Domain { any, positive, nonnegative, nonzero, angle, efficiency };
#ifdef PHYSIK_DATA_CATALOG
struct Input {
  uint8_t domain;
  constexpr Input(const char*,const char*,const char*,Domain d=any):domain(static_cast<uint8_t>(d)){}
};
#else
struct Input { const char* name; const char* unit; const char* initial; Domain domain=any; };
#endif
enum Op {
  fieldE,fieldF,fieldQ,coulombF,coulombR,coulombQ,coulombEr,pointE,pointQ,pointR,superpose,
  capState,capGeometry,capArea,capDistance,capEr,capConnected,capIsolated,capSeries,capParallel,capEnergy,
  rcState,rcConstants,rcTimeU,rcTimeEnergy,rcTimeQ,rcTimeI,rcTimeWPos,rcTimeWNeg,rcTimeLoss,
  wireF,wireB,wireI,wireLength,wireAngle,chargeF,chargeB,chargeQ,chargeV,chargeAngle,
  wireVector,chargeVector,solenoidB,solenoidI,solenoidN,solenoidMu,solenoidLength,longwireB,longwireI,longwireR,
  parallelF,parallelI,wireR,wireLengthR,coilWire,winding,windingH,ring,ringWinding,ringWindingI,ringWindingArea,ringCurrent,
  coilNL,coilNH,coilLinkage,
  orbit,orbitB,orbitU,orbitQM,orbitSpeed,particleV,particleU,particleM,particleQ,particleBT,particleBF,particleQMT,particleVR,transverse,transverseAcc,
  wienV,wienE,wienB,massspec,massspecR,cyclotron,cyclotronTurns,millikanR,millikanM,
  flux,fluxB,fluxA,induction,inductionStates,moving,self,selfL,coilL,
  rlOn,rlOff,rlOffInitial,rlTimeOn,rlTimeOff,
  trafoIdle,trafoN2,trafoN1,trafoU1,trafoLoad,trafoEta,trafoFromV,trafoEfficiency,trafoI2
};
struct Definition {
#ifdef PHYSIK_DATA_CATALOG
  const char* id;
  uint64_t domains;
  uint8_t op,min,max,arity,stride;
  constexpr Definition(const char* key,const char*,const char*,const char*,Op kind,
                       std::initializer_list<Input> args,unsigned lo=0,unsigned hi=0,
                       std::initializer_list<Input> more={}):id(key),domains(0),op(static_cast<uint8_t>(kind)),
    min(static_cast<uint8_t>(lo)),max(static_cast<uint8_t>(hi)),arity(static_cast<uint8_t>(args.size())),stride(static_cast<uint8_t>(more.size())){
    unsigned index=0;
    for(const auto& p:args)domains|=static_cast<uint64_t>(p.domain)<<(3*index++);
    for(const auto& p:more)domains|=static_cast<uint64_t>(p.domain)<<(3*index++);
  }
#else
  const char* id; const char* category; const char* title; const char* formula;
  Op op; std::initializer_list<Input> inputs;
  unsigned min=0,max=0;
  std::initializer_list<Input> repeated={};
#endif
};
// The table is also the direct-call validation contract. All numerical inputs are SI.
#ifdef PHYSIK_DATA_CATALOG
constexpr Definition definitions[]={
#else
const Definition definitions[]={
#endif
  {"E_FIELD_E","E_FIELD","Feldstaerke aus Kraft","E=F/q",fieldE,{{"F","N","1"},{"q","C","1e-6",nonzero}}},
  {"E_FIELD_F","E_FIELD","Kraft im E-Feld","F=q E",fieldF,{{"E","V/m","1000"},{"q","C","1e-6"}}},
  {"E_FIELD_Q","E_FIELD","Ladung aus Feldkraft","q=F/E",fieldQ,{{"F","N","1"},{"E","V/m","1000",nonzero}}},
  {"E_COULOMB_F","E_FIELD","Coulombkraft (mit Vorzeichen)","F=q1 q2/(4 pi eps0 er r^2)",coulombF,{{"q1","C","1e-6"},{"q2","C","-1e-6"},{"r","m","0.1",positive},{"er","","1",positive}}},
  {"E_COULOMB_R","E_FIELD","Coulomb: Abstand","r=sqrt(q1 q2/(4 pi eps0 er F))",coulombR,{{"q1","C","1e-6",nonzero},{"q2","C","-1e-6",nonzero},{"F radial","N","-0.9",nonzero},{"er","","1",positive}}},
  {"E_COULOMB_Q","E_FIELD","Coulomb: Ladung q1","q1=4 pi eps0 er F r^2/q2",coulombQ,{{"F radial","N","-0.9"},{"q2","C","-1e-6",nonzero},{"r","m","0.1",positive},{"er","","1",positive}}},
  {"E_COULOMB_ER","E_FIELD","Coulomb: Dielektrikum","er=q1 q2/(4 pi eps0 F r^2)",coulombEr,{{"q1","C","1e-6",nonzero},{"q2","C","-1e-6",nonzero},{"F radial","N","-0.9",nonzero},{"r","m","0.1",positive}}},
  {"E_POINT_E","E_FIELD","Punktladung: Feld und Potential","E=k Q/r^2; Phi=k Q/r",pointE,{{"Q","C","70e-9"},{"r","m","0.07",positive},{"er","","3.9",positive}}},
  {"E_POINT_Q","E_FIELD","Punktladung: Ladung","Q=4 pi eps0 er E r^2",pointQ,{{"E radial","V/m","1000"},{"r","m","0.1",positive},{"er","","1",positive}}},
  {"E_POINT_R","E_FIELD","Punktladung: Abstand","r=sqrt(Q/(4 pi eps0 er E))",pointR,{{"Q","C","1e-6",nonzero},{"E radial","V/m","1000",nonzero},{"er","","1",positive}}},
  {"E_SUPERPOSITION","E_FIELD","Punktladungen in 2D/3D","E=sum(k Qi (P-ri)/|P-ri|^3); F=qTest E",superpose,{{"Anzahl","","2",positive},{"er","","1",positive},{"x P","m","0"},{"y P","m","1"},{"z P","m","0"},{"qTest (0: nur Feld)","C","0"}},1,8,{{"Q","C","1e-6"},{"x Q","m","0"},{"y Q","m","0"},{"z Q","m","0"}}},
  {"E_CAP_STATE","E_CAP","Kondensator: Ladung und Energie","Q=C U; W=C U^2/2",capState,{{"C","F","1.5e-6",positive},{"U","V","220"}}},
  {"E_CAP_GEOMETRY","E_CAP","Plattenkondensator mit Dielektrikum","C=eps0 er A/d; E=U/d",capGeometry,{{"A","m^2","0.02",positive},{"d","m","0.008",positive},{"er","","1",positive},{"U","V","400"}}},
  {"E_CAP_AREA","E_CAP","Kondensator: Flaeche","A=C d/(eps0 er)",capArea,{{"C","F","1e-9",positive},{"d","m","0.001",positive},{"er","","1",positive}}},
  {"E_CAP_DISTANCE","E_CAP","Kondensator: Abstand","d=eps0 er A/C",capDistance,{{"C","F","1e-9",positive},{"A","m^2","0.02",positive},{"er","","1",positive}}},
  {"E_CAP_ER","E_CAP","Kondensator: Dielektrikum","er=C d/(eps0 A)",capEr,{{"C","F","1e-9",positive},{"A","m^2","0.02",positive},{"d","m","0.001",positive}}},
  {"E_CAP_CONNECTED","E_CAP","Aenderung: Quelle angeschlossen","U1=U0; Q1=C1 U0",capConnected,{{"A0","m^2","0.02",positive},{"d0","m","0.008",positive},{"er0","","1",positive},{"U0","V","400"},{"A1","m^2","0.02",positive},{"d1","m","0.02",positive},{"er1","","1",positive}}},
  {"E_CAP_ISOLATED","E_CAP","Aenderung: Quelle getrennt","Q1=Q0; U1=Q0/C1",capIsolated,{{"A0","m^2","0.02",positive},{"d0","m","0.008",positive},{"er0","","1",positive},{"U0","V","400"},{"A1","m^2","0.02",positive},{"d1","m","0.02",positive},{"er1","","1",positive}}},
  {"E_CAP_SERIES","E_CAP","Kondensatoren in Reihe (2-8)","1/C=sum(1/Ci); Ui=Q/Ci",capSeries,{{"Anzahl","","2",positive},{"U gesamt","V","110"}},2,8,{{"C","F","1.5e-6",positive}}},
  {"E_CAP_PARALLEL","E_CAP","Kondensatoren parallel (2-8)","C=sum(Ci); Qi=Ci U",capParallel,{{"Anzahl","","2",positive},{"U gesamt","V","110"}},2,8,{{"C","F","1.5e-6",positive}}},
  {"E_CAP_U_ENERGY","E_CAP","Spannungsbetrag aus Energie","|U|=sqrt(2 W/C)",capEnergy,{{"C","F","50e-6",positive},{"W","J","300",nonnegative}}},
  {"E_RC_STATE","E_RC","RC: beliebige Anfangs-/Quellspannung","U=Us+(U0-Us) exp(-t/(R C))",rcState,{{"C","F","50e-6",positive},{"R","Ohm","100",positive},{"U0","V","4000"},{"Us","V","0"},{"t","s","0.005",nonnegative}}},
  {"E_RC_CONSTANTS","E_RC","RC: Zeit- und Halbwertszeiten","tau=R C; tHalfU=tau ln2; tHalfW=tau ln2/2",rcConstants,{{"C","F","50e-6",positive},{"R","Ohm","100",positive}}},
  {"E_RC_TIME_U","E_RC","RC: Zeit bis Zielspannung","t=-R C ln((Uziel-Us)/(U0-Us))",rcTimeU,{{"C","F","50e-6",positive},{"R","Ohm","100",positive},{"U0","V","4000"},{"Us","V","0"},{"Uziel","V","2000"}}},
  {"E_RC_TIME_ENERGY","E_RC","RC-Entladung: Zeit bis Zielenergie","t=-R C ln(Wziel/W0)/2",rcTimeEnergy,{{"C","F","50e-6",positive},{"R","Ohm","100",positive},{"U0","V","4000",nonzero},{"Wziel","J","200",positive}}},
  {"E_RC_TIME_Q","E_RC","RC: Zeit bis Zielladung","Uziel=Qziel/C; t=-tau ln((Uziel-Us)/(U0-Us))",rcTimeQ,{{"C","F","50e-6",positive},{"R","Ohm","100",positive},{"U0","V","4000"},{"Us","V","0"},{"Qziel","C","0.1"}}},
  {"E_RC_TIME_I","E_RC","RC: Zeit bis Zielstrom","Uziel=Us-R Iziel; I positiv zum Kondensator",rcTimeI,{{"C","F","50e-6",positive},{"R","Ohm","100",positive},{"U0","V","4000"},{"Us","V","0"},{"Iziel","A","-20"}}},
  {"E_RC_TIME_W_POS","E_RC","RC Zielenergie: positive Zielspannung","Uziel=+sqrt(2 Wziel/C)",rcTimeWPos,{{"C","F","1",positive},{"R","Ohm","2",positive},{"U0","V","-3"},{"Us","V","5"},{"Wziel","J","0.5",nonnegative}}},
  {"E_RC_TIME_W_NEG","E_RC","RC Zielenergie: negative Zielspannung","Uziel=-sqrt(2 Wziel/C)",rcTimeWNeg,{{"C","F","1",positive},{"R","Ohm","2",positive},{"U0","V","-3"},{"Us","V","5"},{"Wziel","J","0.5",nonnegative}}},
  {"E_RC_TIME_LOSS","E_RC","RC: Zeit bis Verlustenergie","t=-tau ln(1-Wverlust/(C (U0-Us)^2/2))/2",rcTimeLoss,{{"C","F","50e-6",positive},{"R","Ohm","100",positive},{"U0","V","4000"},{"Us","V","0"},{"Wverlust","J","200",nonnegative}}},
  {"E_WIRE_F","E_MAG","Leiterkraft mit Winkel","|F|=|B I l sin(alpha)|",wireF,{{"B","T","0.82",nonnegative},{"I","A","18"},{"l","m","0.128",positive},{"alpha","rad","90",angle}}},
  {"E_WIRE_B","E_MAG","Leiterkraft: B","B=F/|I l sin(alpha)|",wireB,{{"F","N","1.88928",nonnegative},{"I","A","18",nonzero},{"l","m","0.128",positive},{"alpha","rad","90",angle}}},
  {"E_WIRE_I","E_MAG","Leiterkraft: Strombetrag","|I|=F/|B l sin(alpha)|",wireI,{{"F","N","1.88928",nonnegative},{"B","T","0.82",positive},{"l","m","0.128",positive},{"alpha","rad","90",angle}}},
  {"E_WIRE_LENGTH","E_MAG","Leiterkraft: Laenge","l=F/|B I sin(alpha)|",wireLength,{{"F","N","1.65",positive},{"B","T","0.34",positive},{"I","A","14.5",nonzero},{"alpha","rad","38",angle}}},
  {"E_WIRE_ANGLE","E_MAG","Leiterkraft: spitzer Winkel","alpha=asin(F/|B I l|)",wireAngle,{{"F","N","1",nonnegative},{"B","T","0.82",positive},{"I","A","18",nonzero},{"l","m","0.128",positive}}},
  {"E_CHARGE_F","E_MAG","Lorentzkraft mit Winkel","|F|=|q| v B sin(alpha)",chargeF,{{"q","C","-1.602176634e-19"},{"v","m/s","1e7",nonnegative},{"B","T","0.001",nonnegative},{"alpha","rad","90",angle}}},
  {"E_CHARGE_B","E_MAG","Lorentzkraft: B","B=F/(|q| v sin(alpha))",chargeB,{{"F","N","1e-15",nonnegative},{"q","C","-1.602176634e-19",nonzero},{"v","m/s","1e7",positive},{"alpha","rad","90",angle}}},
  {"E_CHARGE_Q","E_MAG","Lorentzkraft: Ladungsbetrag","|q|=F/(v B sin(alpha))",chargeQ,{{"F","N","1e-15",nonnegative},{"v","m/s","1e7",positive},{"B","T","0.001",positive},{"alpha","rad","90",angle}}},
  {"E_CHARGE_V","E_MAG","Lorentzkraft: Geschwindigkeit","v=F/(|q| B sin(alpha))",chargeV,{{"F","N","1e-15",nonnegative},{"q","C","-1.602176634e-19",nonzero},{"B","T","0.001",positive},{"alpha","rad","90",angle}}},
  {"E_CHARGE_ANGLE","E_MAG","Lorentzkraft: spitzer Winkel","alpha=asin(F/(|q| v B))",chargeAngle,{{"F","N","1e-15",nonnegative},{"q","C","-1.602176634e-19",nonzero},{"v","m/s","1e7",positive},{"B","T","0.001",positive}}},
  {"E_WIRE_VECTOR","E_MAG","Leiterkraft: Richtung in 3D","F=I (l x B)",wireVector,{{"I vorzeichenbehaftet","A","2"},{"lx","m","0.1"},{"ly","m","0"},{"lz","m","0"},{"Bx","T","0"},{"By","T","0"},{"Bz","T","1"}}},
  {"E_CHARGE_VECTOR","E_MAG","Lorentzkraft: Richtung in 3D","F=q (v x B)",chargeVector,{{"q vorzeichenbehaftet","C","-1.602176634e-19"},{"vx","m/s","1e7"},{"vy","m/s","0"},{"vz","m/s","0"},{"Bx","T","0"},{"By","T","0"},{"Bz","T","0.001"}}},
  {"E_SOLENOID_B","E_MAG","Lange Spule: H und B","H=N I/l; B=mu0 mur H",solenoidB,{{"mur","","1",positive},{"N","","820",positive},{"I","A","1"},{"l","m","0.64",positive}}},
  {"E_SOLENOID_I","E_MAG","Lange Spule: Strom","I=B l/(mu0 mur N)",solenoidI,{{"B","T","0.0012"},{"mur","","1",positive},{"N","","820",positive},{"l","m","0.64",positive}}},
  {"E_SOLENOID_N","E_MAG","Lange Spule: Windungszahl","N=B l/(mu0 mur I)",solenoidN,{{"B","T","0.0012",nonzero},{"mur","","1",positive},{"I","A","1",nonzero},{"l","m","0.64",positive}}},
  {"E_SOLENOID_MU","E_MAG","Lange Spule: Permeabilitaet","mur=B l/(mu0 N I)",solenoidMu,{{"B","T","1.5",nonzero},{"N","","500",positive},{"I","A","1.2",nonzero},{"l","m","0.24",positive}}},
  {"E_SOLENOID_LENGTH","E_MAG","Lange Spule: Laenge","l=mu0 mur N I/B",solenoidLength,{{"B","T","0.0012",nonzero},{"mur","","1",positive},{"N","","820",positive},{"I","A","1",nonzero}}},
  {"E_LONGWIRE_B","E_MAG","Langer gerader Leiter: B","B=mu0 mur I/(2 pi r)",longwireB,{{"mur","","1",positive},{"I","A","1"},{"r","m","1",positive}}},
  {"E_LONGWIRE_I","E_MAG","Langer Leiter: Strom","I=2 pi r B/(mu0 mur)",longwireI,{{"B","T","2e-7"},{"mur","","1",positive},{"r","m","1",positive}}},
  {"E_LONGWIRE_R","E_MAG","Langer Leiter: Abstand","r=mu0 mur I/(2 pi B)",longwireR,{{"B","T","2e-7",nonzero},{"mur","","1",positive},{"I","A","1",nonzero}}},
  {"E_PARALLEL_F","E_MAG","Parallele Leiter: Kraft","F/l=-mu0 mur I1 I2/(2 pi d)",parallelF,{{"I1","A","1"},{"I2","A","1"},{"d","m","1",positive},{"l","m","1",positive},{"mur","","1",positive}}},
  {"E_PARALLEL_I","E_MAG","Parallele Leiter: Strom I1","I1=-2 pi d F/(mu0 mur I2 l)",parallelI,{{"F radial","N","-0.0034"},{"I2","A","58",nonzero},{"d","m","0.12",positive},{"l","m","2.8",positive},{"mur","","1",positive}}},
  {"E_WIRE_RESISTANCE","E_MAG","Drahtwiderstand","A=pi d^2/4; R=rho l/A",wireR,{{"rho Material","Ohm*m","1.78e-8",positive},{"Drahtlaenge","m","10",positive},{"Drahtdurchmesser","m","0.0005",positive}}},
  {"E_WIRE_LENGTH_R","E_MAG","Drahtlaenge aus Widerstand","l=R pi d^2/(4 rho)",wireLengthR,{{"R","Ohm","1",positive},{"rho Material","Ohm*m","1.78e-8",positive},{"Drahtdurchmesser","m","0.0005",positive}}},
  {"E_COIL_WIRE","E_MAG","Spule aus Drahtdaten (N gegeben)","R=rho pi D N/Awire; L=mu0 mur N^2 A/l",coilWire,{{"N","","100",positive},{"Spulenlaenge","m","0.065",positive},{"mittlerer Windungsdurchmesser","m","0.006",positive},{"Drahtdurchmesser","m","0.0005",positive},{"rho Material","Ohm*m","1.78e-8",positive},{"mur fest","","1",positive},{"U","V","10"}}},
  {"E_WINDING","E_MAG","Zylinderspule: dichte Wicklung","N=Lagen floor(l/d); Drahtlaenge=sum(pi Dlage Nlage)",winding,{{"Spulenlaenge","m","0.3",positive},{"Kerndurchmesser","m","0.05",positive},{"Drahtdurchmesser","m","0.0008",positive},{"Lagen (1-40)","","1",positive},{"rho Material","Ohm*m","1.78e-8",positive},{"mur fest","","880",positive},{"U","V","20"}}},
  {"E_WINDING_H","E_MAG","Dichte lange Wicklung: H und B","H=Lagen I/d; B=mu0 mur H",windingH,{{"Drahtdurchmesser","m","0.0002",positive},{"Lagen (1-40)","","2",positive},{"I","A","0.52"},{"mur fest","","1",positive}}},
  {"E_RING","E_MAG","Ringkern: N und Strom gegeben","H=N I/lm; L=mu0 mur N^2 A/lm",ring,{{"N","","450",positive},{"Innenradius","m","0.04",positive},{"Aussenradius","m","0.06",positive},{"I","A","1"},{"mur fest","","1",positive}}},
  {"E_RING_WINDING","E_MAG","Ringkern: dichte Wicklung und U","N=floor(2 pi ri/d); I=U/R; Uind=-L dI/dt",ringWinding,{{"Innenradius","m","0.2",positive},{"Aussenradius","m","0.25",positive},{"Drahtdurchmesser","m","0.0012",positive},{"rho Material","Ohm*m","1.78e-8",positive},{"mur fest","","400",positive},{"U","V","10"},{"Ausschaltzeit","s","0.002",positive}}},
  {"E_RING_WINDING_I","E_MAG","Ringkern: dichte Wicklung und Strom","N=floor(2 pi ri/d); H=N I/lm",ringWindingI,{{"Innenradius","m","0.2",positive},{"Aussenradius","m","0.25",positive},{"Drahtdurchmesser","m","0.0012",positive},{"I","A","2.5"},{"mur fest","","400",positive}}},
  {"E_RING_WINDING_AREA","E_MAG","Ringkern: Drahtquerschnitt und U","d=sqrt(4 Awire/pi); N=floor(2 pi ri/d)",ringWindingArea,{{"Innenradius","m","0.6",positive},{"Aussenradius","m","1.25",positive},{"Drahtquerschnitt","m^2","1e-5",positive},{"rho Material","Ohm*m","4.9e-7",positive},{"mur fest","","250",positive},{"U","V","310"},{"Ausschaltzeit","s","0.002",positive}}},
  {"E_RING_CURRENT","E_MAG","Ringkern: Strom aus Fluss","I=Phi lm/(mu0 mur N A)",ringCurrent,{{"N","","450",positive},{"Innenradius","m","0.04",positive},{"Aussenradius","m","0.06",positive},{"mur fest","","1",positive},{"Phi","Wb","2e-6"}}},
  {"E_COIL_N_L","E_MAG","Spule: N aus Induktivitaet","N=sqrt(L l/(mu0 mur A))",coilNL,{{"L","H","0.05",positive},{"A","m^2","2.827433388e-5",positive},{"mur fest","","1",positive},{"l","m","0.06",positive}}},
  {"E_COIL_N_H","E_MAG","Spule: N und Laenge aus L,H,I","N=L I/(mu0 mur A H); l=N I/H",coilNH,{{"L","H","1.5",positive},{"A","m^2","0.0005",positive},{"mur fest","","1900",positive},{"H","A/m","500",nonzero},{"I","A","0.3",nonzero}}},
  {"E_COIL_LINKAGE","E_MAG","Spule: N aus L,I,Phi","N=L I/Phi",coilLinkage,{{"L","H","0.06",positive},{"I","A","5",nonzero},{"Phi","Wb","0.015",nonzero}}},
  {"E_PARTICLE_ORBIT","E_PARTICLE","Beschleunigungsspannung und Kreisbahn","v=sqrt(2 |q| U/m); r=m v/(|q| B)",orbit,{{"Uacc Betrag","V","200",positive},{"q","C","-1.602176634e-19",nonzero},{"m","kg","9.1093837139e-31",positive},{"B Betrag","T","0.00125",positive}}},
  {"E_PARTICLE_B","E_PARTICLE","Kreisbahn: Magnetfeld","B=sqrt(2 m U/|q|)/r",orbitB,{{"Uacc Betrag","V","200",positive},{"q","C","-1.602176634e-19",nonzero},{"m","kg","9.1093837139e-31",positive},{"r","m","0.039",positive}}},
  {"E_PARTICLE_U","E_PARTICLE","Kreisbahn: Beschleunigungsspannung","U=|q| B^2 r^2/(2 m)",orbitU,{{"r","m","0.039",positive},{"B Betrag","T","0.00125",positive},{"q","C","-1.602176634e-19",nonzero},{"m","kg","9.1093837139e-31",positive}}},
  {"E_PARTICLE_QM","E_PARTICLE","Fadenstrahlrohr: spezifische Ladung","|q|/m=2 U/(B r)^2",orbitQM,{{"Uacc Betrag","V","200",positive},{"B Betrag","T","0.00125",positive},{"r","m","0.039",positive}}},
  {"E_PARTICLE_SPEED","E_PARTICLE","Kreisbahn aus Geschwindigkeit","r=m v/(|q| B); T=2 pi m/(|q| B)",orbitSpeed,{{"v","m/s","1e7",positive},{"q","C","-1.602176634e-19",nonzero},{"m","kg","9.1093837139e-31",positive},{"B Betrag","T","0.00125",positive}}},
  {"E_PARTICLE_V","E_PARTICLE","Geschwindigkeit aus Spannung","v=sqrt(2 |q| U/m)",particleV,{{"Uacc Betrag","V","200",nonnegative},{"q","C","-1.602176634e-19",nonzero},{"m","kg","9.1093837139e-31",positive}}},
  {"E_PARTICLE_U_V","E_PARTICLE","Beschleunigungsspannung aus v","U=m v^2/(2 |q|)",particleU,{{"v","m/s","1e7",nonnegative},{"q","C","-1.602176634e-19",nonzero},{"m","kg","9.1093837139e-31",positive}}},
  {"E_PARTICLE_M","E_PARTICLE","Kreisbahn: Masse","m=|q| (B r)^2/(2 U)",particleM,{{"Uacc Betrag","V","200",positive},{"q","C","-1.602176634e-19",nonzero},{"B Betrag","T","0.00125",positive},{"r","m","0.039",positive}}},
  {"E_PARTICLE_Q","E_PARTICLE","Kreisbahn: Ladungsbetrag","|q|=2 m U/(B r)^2",particleQ,{{"Uacc Betrag","V","200",positive},{"m","kg","9.1093837139e-31",positive},{"B Betrag","T","0.00125",positive},{"r","m","0.039",positive}}},
  {"E_PARTICLE_B_T","E_PARTICLE","Kreisbahn: B aus Umlaufzeit","B=2 pi m/(|q| T)",particleBT,{{"T","s","1e-8",positive},{"q","C","-1.602176634e-19",nonzero},{"m","kg","9.1093837139e-31",positive}}},
  {"E_PARTICLE_B_F","E_PARTICLE","Kreisbahn: B aus Frequenz","B=2 pi m f/|q|",particleBF,{{"f","Hz","1e8",positive},{"q","C","-1.602176634e-19",nonzero},{"m","kg","9.1093837139e-31",positive}}},
  {"E_PARTICLE_QM_T","E_PARTICLE","Spezifische Ladung aus Umlaufzeit","|q|/m=2 pi/(B T)",particleQMT,{{"B Betrag","T","0.00125",positive},{"T","s","1e-8",positive}}},
  {"E_PARTICLE_V_R","E_PARTICLE","Kreisbahn: v aus Radius","v=|q| B r/m",particleVR,{{"r","m","0.039",positive},{"B Betrag","T","0.00125",positive},{"q","C","-1.602176634e-19",nonzero},{"m","kg","9.1093837139e-31",positive}}},
  {"E_TRANSVERSE","E_PARTICLE","Elektrisches Querfeld und Schirm","ay=q U/(m d); y=ay t^2/2",transverse,{{"q","C","-1.602176634e-19",nonzero},{"m","kg","9.1093837139e-31",positive},{"vx","m/s","1e7",positive},{"U quer (E in +y)","V","60"},{"Plattenabstand","m","0.01",positive},{"Plattenlaenge","m","0.03",positive},{"Schirmabstand ab Austritt","m","0.2",nonnegative}}},
  {"E_TRANSVERSE_ACC","E_PARTICLE","Querfeld nach Beschleunigung","vx=sqrt(2 |q| Uacc/m); Plattenmitte als Start",transverseAcc,{{"q","C","-1.602176634e-19",nonzero},{"m","kg","9.1093837139e-31",positive},{"Uacc Betrag","V","200",positive},{"U quer (E in +y)","V","60"},{"Plattenabstand","m","0.01",positive},{"Plattenlaenge","m","0.03",positive},{"Schirmabstand ab Austritt","m","0.2",nonnegative}}},
  {"E_WIEN_V","E_PARTICLE","Wien-Filter: Geschwindigkeit","v=E/B",wienV,{{"E Betrag","V/m","1500",positive},{"B Betrag","T","0.0015",positive}}},
  {"E_WIEN_E","E_PARTICLE","Wien-Filter: E","E=v B",wienE,{{"v","m/s","1e6",positive},{"B Betrag","T","0.0015",positive}}},
  {"E_WIEN_B","E_PARTICLE","Wien-Filter: B","B=E/v",wienB,{{"E Betrag","V/m","1500",positive},{"v","m/s","1e6",positive}}},
  {"E_MASSSPEC","E_PARTICLE","Massenspektrometer: Masse","v=Uf/(d Bf); m=|q| Bd r/v",massspec,{{"U Filter Betrag","V","60",positive},{"d Filter","m","0.04",positive},{"B Filter Betrag","T","0.0015",positive},{"B Ablenkung Betrag","T","0.1",positive},{"r","m","0.1",positive},{"q","C","1.602176634e-19",nonzero}}},
  {"E_MASSSPEC_R","E_PARTICLE","Massenspektrometer: Radius","r=m Uf/(|q| Bd d Bf)",massspecR,{{"m","kg","6.64e-27",positive},{"q","C","3.204353268e-19",nonzero},{"U Filter Betrag","V","60",positive},{"d Filter","m","0.04",positive},{"B Filter Betrag","T","0.0015",positive},{"B Ablenkung Betrag","T","0.1",positive}}},
  {"E_CYCLOTRON","E_PARTICLE","Zyklotron: zwei Spalte pro Umlauf","W=2 n |q| Uspalt; f=|q| B/(2 pi m)",cyclotron,{{"q","C","1.602176634e-19",nonzero},{"m","kg","1.67262192369e-27",positive},{"B Betrag","T","1",positive},{"Uspalt Betrag","V","1000",positive},{"Umlaeufe ganzzahlig","","100",positive}}},
  {"E_CYCLOTRON_TURNS","E_PARTICLE","Zyklotron: Zielradius","n=|q| B^2 r^2/(4 m Uspalt)",cyclotronTurns,{{"q","C","1.602176634e-19",nonzero},{"m","kg","1.67262192369e-27",positive},{"B Betrag","T","1",positive},{"Uspalt Betrag","V","1000",positive},{"r","m","0.1",positive}}},
  {"E_MILLIKAN_RADIUS","E_PARTICLE","Millikan: Radius und Auftrieb","q=(rho-rhoLuft) 4 pi r^3 g d/(3 U)",millikanR,{{"r","m","1e-6",positive},{"rho Tropfen","kg/m^3","2300",positive},{"rho Luft (0: ohne)","kg/m^3","0",nonnegative},{"d","m","0.001",positive},{"U (Feld nach oben)","V","115",nonzero},{"g","m/s^2","9.81",positive}}},
  {"E_MILLIKAN_MASS","E_PARTICLE","Millikan: Masse und Auftrieb","q=(m-rhoLuft m/rho) g d/U",millikanM,{{"m","kg","9.634217471e-15",positive},{"rho Tropfen","kg/m^3","2300",positive},{"rho Luft (0: ohne)","kg/m^3","0",nonnegative},{"d","m","0.001",positive},{"U (Feld nach oben)","V","115",nonzero},{"g","m/s^2","9.81",positive}}},
  {"E_FLUX","E_IND","Magnetischer Fluss","Phi=B A cos(alpha zur Flaechennormalen)",flux,{{"B","T","0.1"},{"A","m^2","0.01",positive},{"alpha zur Normalen","rad","0",angle}}},
  {"E_FLUX_B","E_IND","Fluss: Magnetfeld","B=Phi/(A cos(alpha))",fluxB,{{"Phi","Wb","0.001"},{"A","m^2","0.01",positive},{"alpha zur Normalen","rad","0",angle}}},
  {"E_FLUX_A","E_IND","Fluss: Flaeche","A=Phi/(B cos(alpha))",fluxA,{{"Phi","Wb","0.001",nonzero},{"B","T","0.1",nonzero},{"alpha zur Normalen","rad","0",angle}}},
  {"E_INDUCTION_MEAN","E_IND","Mittlere orientierte Induktion","U=-N (Phi1-Phi0)/dt",induction,{{"N","","100",positive},{"Phi0 pro Windung","Wb","0"},{"Phi1 pro Windung","Wb","0.001"},{"dt","s","0.1",positive}}},
  {"E_INDUCTION_STATES","E_IND","Induktion aus zwei Feldzustaenden","U=-N (B1 A1 cos(a1)-B0 A0 cos(a0))/dt",inductionStates,{{"N","","100",positive},{"B0","T","0.1"},{"A0","m^2","0.01",positive},{"a0 zur Normalen","rad","0",angle},{"B1","T","0.1"},{"A1","m^2","0.01",positive},{"a1 zur Normalen","rad","90",angle},{"dt","s","0.1",positive}}},
  {"E_MOVING_CONDUCTOR","E_IND","Bewegter Leiter: allgemeine Richtung","U=(v x B) dot l",moving,{{"vx","m/s","0.5"},{"vy","m/s","0"},{"vz","m/s","0"},{"Bx","T","0"},{"By","T","0"},{"Bz","T","0.04"},{"lx","m","0"},{"ly","m","-1.5"},{"lz","m","0"}}},
  {"E_SELF","E_IND","Selbstinduktion: mittlere Spannung","Uind=-L (I1-I0)/dt",self,{{"L","H","0.9",positive},{"I0","A","0"},{"I1","A","0.03636363636"},{"dt","s","0.002",positive}}},
  {"E_SELF_L","E_IND","Selbstinduktion: Induktivitaet","L=-Uind dt/(I1-I0)",selfL,{{"Uind","V","-12",nonzero},{"I0","A","0"},{"I1","A","0.04"},{"dt","s","0.003",positive}}},
  {"E_COIL_L","E_IND","Induktivitaet der langen Spule","L=mu0 mur N^2 A/l",coilL,{{"mur fest","","1",positive},{"N","","850",positive},{"A","m^2","0.0002864788976",positive},{"l","m","0.15",positive}}},
  {"E_RL_ON","E_RL","RL Einschalten: Anfangsstrom frei","I=U/R+(I0-U/R) exp(-R t/L)",rlOn,{{"L","H","0.9",positive},{"R gesamt","Ohm","330",positive},{"U","V","12"},{"I0","A","0"},{"t","s","0.002727272727",nonnegative}}},
  {"E_RL_OFF","E_RL","RL Ausschalten: eigener Entladewiderstand","I0=U/Ran; I=I0 exp(-Raus t/L)",rlOff,{{"L","H","0.9",positive},{"R Einschalten","Ohm","330",positive},{"R Entladekreis gesamt","Ohm","100",positive},{"U vor Ausschalten","V","12"},{"t","s","0.009",nonnegative}}},
  {"E_RL_OFF_INITIAL","E_RL","RL geschlossener Entladekreis: I0","I=I0 exp(-Raus t/L)",rlOffInitial,{{"L","H","0.9",positive},{"R Entladekreis gesamt","Ohm","100",positive},{"I0","A","0.03636363636"},{"t","s","0.009",nonnegative}}},
  {"E_RL_TIME_ON","E_RL","RL Einschalten: Zielstromzeit","t=-L/R ln((Iziel-U/R)/(I0-U/R))",rlTimeOn,{{"L","H","0.9",positive},{"R gesamt","Ohm","330",positive},{"U","V","12"},{"I0","A","0"},{"Iziel","A","0.01818181818"}}},
  {"E_RL_TIME_OFF","E_RL","RL Ausschalten: Zielstromzeit","t=-L/Raus ln(Iziel/I0)",rlTimeOff,{{"L","H","0.9",positive},{"R Entladekreis gesamt","Ohm","100",positive},{"I0","A","0.03636363636",nonzero},{"Iziel","A","0.01818181818"}}},
  {"E_TRAFO_IDLE","E_TRAFO","Transformator: idealer Leerlauf","U2/U1=N2/N1; I1=I2=0",trafoIdle,{{"U1 effektiv","V","230",positive},{"N1","","5500",positive},{"N2","","200",positive}}},
  {"E_TRAFO_N2","E_TRAFO","Transformator: Sekundaerwindungen","N2=N1 U2/U1",trafoN2,{{"U1 effektiv","V","230",positive},{"U2 effektiv","V","8",positive},{"N1","","5500",positive}}},
  {"E_TRAFO_N1","E_TRAFO","Transformator: Primaerwindungen","N1=N2 U1/U2",trafoN1,{{"U1 effektiv","V","230",positive},{"U2 effektiv","V","8",positive},{"N2","","200",positive}}},
  {"E_TRAFO_U1","E_TRAFO","Transformator: Primaerspannung","U1=U2 N1/N2",trafoU1,{{"U2 effektiv","V","8",positive},{"N1","","5500",positive},{"N2","","200",positive}}},
  {"E_TRAFO_LOAD","E_TRAFO","Transformator: ideal belastet","U2=U1 N2/N1; U1 I1=U2 I2",trafoLoad,{{"U1 effektiv","V","230",positive},{"N1","","5500",positive},{"N2","","200",positive},{"I2 effektiv","A","2.6",nonnegative}}},
  {"E_TRAFO_ETA","E_TRAFO","Transformator: Last und Wirkungsgrad","U2=U1 N2/N1; P1=P2/eta",trafoEta,{{"U1 effektiv","V","230",positive},{"N1","","5500",positive},{"N2","","200",positive},{"I2 effektiv","A","2.6",nonnegative},{"eta","","0.9",efficiency}}},
  {"E_TRAFO_FROM_V","E_TRAFO","Transformator: Zielspannung und Last","N2=N1 U2/U1; I1=U2 I2/(eta U1)",trafoFromV,{{"U1 effektiv","V","230",positive},{"U2 effektiv","V","8",positive},{"N1","","5500",positive},{"I2 effektiv","A","2.6",nonnegative},{"eta","","1",efficiency}}},
  {"E_TRAFO_EFFICIENCY","E_TRAFO","Transformator: Wirkungsgrad messen","eta=U2 I2/(U1 I1)",trafoEfficiency,{{"U1 effektiv","V","230",positive},{"I1 effektiv","A","0.1",positive},{"U2 effektiv","V","8",positive},{"I2 effektiv","A","2.6",positive}}},
  {"E_TRAFO_I2","E_TRAFO","Transformator: Sekundaerstrom","U2=U1 N2/N1; I2=eta U1 I1/U2",trafoI2,{{"U1 effektiv","V","230",positive},{"N1","","5500",positive},{"N2","","200",positive},{"I1 effektiv","A","0.1",nonnegative},{"eta","","1",efficiency}}}
};

void add(NativeResult& r,const char* name,double value,const char* unit="") {
  r.values.push_back({name,value,unit});
}
bool bad(std::string& e,const char* message="Keine eindeutige physikalische Loesung fuer diese Eingaben.") {
  e=message; return false;
}
double area(double diameter) { return pi*diameter*diameter/4; }
double norm(double x,double y,double z) { return std::hypot(std::hypot(x,y),z); }
double sine(double a) { return a==0 || a==pi ? 0 : std::sin(a); }
double cosine(double a) { return a==pi/2 ? 0 : std::cos(a); }
void vectorResult(NativeResult& r,const char* x,const char* y,const char* z,const char* magnitude,
                  double a,double b,double c,const char* unit) {
  add(r,x,a,unit); add(r,y,b,unit); add(r,z,c,unit); add(r,magnitude,norm(a,b,c),unit);
}
bool classical(double speed,NativeResult& r,std::string& e) {
  if (!std::isfinite(speed) || speed<0 || speed>=light)
    return bad(e,"Klassisches Modell unzulaessig: Geschwindigkeit muss kleiner als c sein.");
  r.note+=" Klassisches Modell ohne Relativitaet.";
  if (speed>=0.1*light) r.note+=" Warnung: v >= 0.1c, relativistische Korrektur erforderlich.";
  return true;
}
void cap(NativeResult& r,double c,double u) {
  add(r,"C",c,"F"); add(r,"U",u,"V"); add(r,"Q",c*u,"C"); add(r,"W",0.5*c*u*u,"J");
}
void constants(NativeResult& r,double tau) {
  add(r,"tau",tau,"s"); add(r,"tHalfU",tau*std::log(2.0),"s");
  add(r,"tHalfW",tau*std::log(2.0)/2,"s");
}
bool targetTime(double initial,double source,double target,double tau,NativeResult& r,std::string& e) {
  if (initial==source) return bad(e,"Stationaerer Zustand: keine eindeutige Zielzeit.");
  const double ratio=(target-source)/(initial-source);
  if (!(ratio>0 && ratio<=1)) return bad(e,"Ziel nur asymptotisch oder nicht in endlicher Zukunft erreichbar.");
  add(r,"t",ratio==1 ? 0 : -tau*std::log(ratio),"s"); return true;
}
double productRatio(std::initializer_list<double> numerator,std::initializer_list<double> denominator){
  double mantissa=1;int exponent=0;
  for(double x:numerator){if(x==0)return 0;int e=0;mantissa*=std::frexp(x,&e);exponent+=e;}
  for(double x:denominator){if(x==0)return std::numeric_limits<double>::quiet_NaN();int e=0;mantissa/=std::frexp(x,&e);exponent-=e;}
  return std::ldexp(mantissa,exponent);
}
double relaxation(double initial,double source,double timeOverTau){
  return initial*std::exp(-timeOverTau)+source*(-std::expm1(-timeOverTau));
}
void circuitPlot(NativeResult& r,double initial,double source,double tau,double duration,bool current) {
  Plot p{current ? "RL Strom" : "RC Spannung","t","s",current ? "I" : "U",current ? "A" : "V",{},{},true};
  const double end=duration>0 ? duration : tau*5;
  for (unsigned i=0;i<=40;++i) {
    const double t=end*(static_cast<double>(i)/40);
    p.points.push_back({t,relaxation(initial,source,t/tau)});
  }
  r.plots.push_back(std::move(p));
}
void coil(NativeResult& r,double n,double length,double a,double mur,double current) {
  const double h=n*current/length,b=mu0*mur*h,l=mu0*mur*n*n*a/length;
  add(r,"N",n); add(r,"I",current,"A"); add(r,"H",h,"A/m"); add(r,"B",b,"T");
  add(r,"Phi",b*a,"Wb"); add(r,"L",l,"H"); add(r,"W",0.5*l*current*current,"J");
}
bool orbitResult(NativeResult& r,double speed,double qm,double b,std::string& e) {
  if (!classical(speed,r,e)) return false;
  r.note+=" Homogenes B, v senkrecht zu B; q/m als Betrag. Uacc als positiver Energiegewinn pro |q| aus Ruhe.";
  add(r,"v",speed,"m/s"); add(r,"q/m",qm,"C/kg"); add(r,"r",speed/(qm*b),"m");
  add(r,"T",2*pi/(qm*b),"s"); add(r,"f",qm*b/(2*pi),"Hz"); return true;
}

bool evaluate(const Definition& d,const std::vector<double>& v,NativeResult& r,std::string& e) {
  const Op op=static_cast<Op>(d.op);
  const double a=v[0],b=v.size()>1?v[1]:0,c=v.size()>2?v[2]:0,f=v.size()>3?v[3]:0;
  switch(op) {
  case fieldE: add(r,"E",a/b,"V/m"); break;
  case fieldF: add(r,"F",a*b,"N"); break;
  case fieldQ: add(r,"q",a/b,"C"); break;
  case coulombF: {
    const double force=productRatio({a,b},{4*pi*eps0,f,c,c});
    add(r,"F radial",force,"N"); add(r,"Betrag",std::abs(force),"N");
    r.note=force<0?"Anziehung: Kraft zur anderen Ladung.":force>0?"Abstossung: Kraft von der anderen Ladung weg.":"Keine Coulombkraft."; break;
  }
  case coulombR: case coulombEr: {
    const double x=a*b/(4*pi*eps0*c*f*(op==coulombEr?f:1));
    if (!(x>0)) return bad(e,"Kraftvorzeichen passt nicht zu den Ladungen (negativ: Anziehung).");
    add(r,op==coulombR?"r":"er",op==coulombR?std::sqrt(x):x,op==coulombR?"m":""); break;
  }
  case coulombQ: add(r,"q1",4*pi*eps0*f*a*c*c/b,"C"); break;
  case pointE: add(r,"E radial",productRatio({a},{4*pi*eps0,c,b,b}),"V/m"); add(r,"Potential",productRatio({a},{4*pi*eps0,c,b}),"V"); break;
  case pointQ: add(r,"Q",4*pi*eps0*c*a*b*b,"C"); break;
  case pointR: {
    const double x=a/(4*pi*eps0*c*b); if (!(x>0)) return bad(e);
    add(r,"r",std::sqrt(x),"m"); break;
  }
  case superpose: {
    double ex=0,ey=0,ez=0,potential=0;
    for (unsigned i=0;i<static_cast<unsigned>(a);++i) {
      const size_t j=6+4*i;
      const double x=c-v[j+1],y=f-v[j+2],z=v[4]-v[j+3],distance=norm(x,y,z);
      if (!(distance>0)) return bad(e,"Auswertungspunkt liegt auf einer Punktladung: Feld ist dort singulaer.");
      const double radial=productRatio({v[j]},{4*pi*eps0,b,distance,distance});
      ex+=radial*(x/distance); ey+=radial*(y/distance); ez+=radial*(z/distance);
      potential+=productRatio({v[j]},{4*pi*eps0,b,distance});
    }
    vectorResult(r,"Ex","Ey","Ez","E",ex,ey,ez,"V/m"); add(r,"Potential",potential,"V");
    vectorResult(r,"Fx","Fy","Fz","F",v[5]*ex,v[5]*ey,v[5]*ez,"N");
    r.note="Homogenes lineares Dielektrikum, Punktladungen. z=0 fuer 2D; qTest=0 fuer reine Feldauswertung."; break;
  }
  case capState: cap(r,a,b); break;
  case capGeometry: cap(r,eps0*c*a/b,f); add(r,"E",f/b,"V/m"); r.note="Ideale parallele Platten, vollstaendiges lineares Dielektrikum; ohne Randfelder."; break;
  case capArea: add(r,"A",a*b/(eps0*c),"m^2"); break;
  case capDistance: add(r,"d",eps0*c*b/a,"m"); break;
  case capEr: add(r,"er",a*c/(eps0*b)); break;
  case capConnected: case capIsolated: {
    const double c0=eps0*c*a/b,c1=eps0*v[6]*v[4]/v[5],u=op==capConnected?f:c0*f/c1;
    add(r,"C0",c0,"F"); add(r,"Q0",c0*f,"C"); add(r,"W0",0.5*c0*f*f,"J"); cap(r,c1,u);
    add(r,"E",u/v[5],"V/m"); add(r,"deltaW",0.5*(c1*u*u-c0*f*f),"J");
    r.note=op==capConnected?"Ideale Spannungsquelle: U bleibt konstant; Q kann fliessen.":"Quelle getrennt: Q bleibt konstant; keine Leckstroeme."; break;
  }
  case capSeries: case capParallel: {
    double sum=0; for (size_t i=2;i<v.size();++i) sum+=op==capSeries?1/v[i]:v[i];
    const double total=op==capSeries?1/sum:sum,q=total*b;
    if (!std::isfinite(sum) || !(total>0)) return bad(e,"Kapazitaet ausserhalb des darstellbaren Zahlenbereichs.");
    cap(r,total,b);
    for (size_t i=2;i<v.size();++i) {
      const std::string suffix=std::to_string(i-1);
      const double u=op==capSeries?q/v[i]:b;
      r.values.push_back({"U"+suffix,u,"V"}); r.values.push_back({"Q"+suffix,v[i]*u,"C"});
    }
    r.note="Ideale Kondensatoren, anfangs ungeladen; in Reihe neutrale Zwischenknoten."; break;
  }
  case capEnergy: add(r,"U Betrag",std::sqrt(2*b/a),"V"); break;
  case rcConstants: constants(r,a*b); r.note="Halbwertszeiten der Spannungsabweichung und ihrer Energie gegen den Endzustand."; break;
  case rcState: {
    const double tau=a*b,t=v[4],decay=std::exp(-t/tau),u=relaxation(c,f,t/tau);
    constants(r,tau); cap(r,a,u); add(r,"I",(f-c)*decay/b,"A");
    add(r,"lostEnergy",0.5*a*(c-f)*(c-f)*(-std::expm1(-2*t/tau)),"J");
    r.note="Ideales RC; I positiv von Quelle zum C. lostEnergy: Waerme im R seit t=0. Halbwerte beziehen sich auf U-Us und C(U-Us)^2/2, nicht auf gesamte Energie bei Us != 0.";
    circuitPlot(r,c,f,tau,t,false); break;
  }
  case rcTimeU: return targetTime(c,f,v[4],a*b,r,e);
  case rcTimeQ: return targetTime(c,f,v[4]/a,a*b,r,e);
  case rcTimeI: {
    // Use the exponential current directly, avoiding cancellation of Us-R*I.
    const double initial=(f-c)/b;
    return targetTime(initial,0,v[4],a*b,r,e);
  }
  case rcTimeWPos: case rcTimeWNeg: {
    const double u=std::sqrt(2*v[4]/a)*(op==rcTimeWNeg?-1:1);
    r.note="Allgemeines RC mit Quelle: Energie kann zweimal erreicht werden. Diese Maske waehlt das angegebene Vorzeichen der Zielspannung.";
    return targetTime(c,f,u,a*b,r,e);
  }
  case rcTimeLoss: {
    const double limit=0.5*a*(c-f)*(c-f),ratio=v[4]/limit;
    if (!(limit>0) || !std::isfinite(limit) || !(ratio>=0 && ratio<1))
      return bad(e,"Verlustziel muss unter der endlichen maximalen Widerstandswaerme liegen; stationaerer Fall nicht eindeutig.");
    add(r,"t",-a*b*std::log1p(-ratio)/2,"s"); break;
  }
  case rcTimeEnergy: {
    const double w0=0.5*a*c*c; if (!(f<=w0)) return bad(e,"Zielenergie ist groesser als die Anfangsenergie.");
    add(r,"t",-a*b*std::log(f/w0)/2,"s"); r.note="Reine Entladung ohne Quelle; beide Spannungsvorzeichen moeglich."; break;
  }
  case wireF: case chargeF: {
    if (op==chargeF && !classical(b,r,e)) return false;
    add(r,"F",std::abs(a*b*c*sine(f)),"N"); break;
  }
  case wireB: case wireI: case wireLength: case chargeB: case chargeQ: case chargeV: {
    const double denominator=std::abs(b*c*sine(f));
    if (!(denominator>0)) return bad(e,"Parallele Richtungen oder Nullfaktor: gesuchte Groesse nicht eindeutig.");
    const double value=a/denominator;
    const char* name=op==wireB||op==chargeB?"B":op==wireI?"I Betrag":op==wireLength?"l":op==chargeQ?"q Betrag":"v";
    const char* unit=op==wireB||op==chargeB?"T":op==wireI?"A":op==wireLength?"m":op==chargeQ?"C":"m/s";
    if ((op==chargeV && !classical(value,r,e)) || (op==chargeB && !classical(c,r,e)) ||
        (op==chargeQ && !classical(b,r,e))) return false;
    add(r,name,value,unit); r.note+=" Ergebnis ist ein Betrag; Richtung separat ueber Vektorrechner."; break;
  }
  case wireAngle: case chargeAngle: {
    if (op==chargeAngle && !classical(c,r,e)) return false;
    const double ratio=a/std::abs(b*c*f); if (!(ratio>=0 && ratio<=1)) return bad(e,"Kraft uebersteigt den maximalen Betrag.");
    add(r,"alpha",std::asin(ratio),"rad"); add(r,"zweiter Winkel",pi-std::asin(ratio),"rad"); break;
  }
  case wireVector: case chargeVector: {
    if (op==chargeVector && !classical(norm(b,c,f),r,e)) return false;
    vectorResult(r,"Fx","Fy","Fz","F",a*(c*v[6]-f*v[5]),a*(f*v[4]-b*v[6]),a*(b*v[5]-c*v[4]),"N");
    r.note+=" Rechtshaendiges xyz-System; negative q bzw. I kehren die Richtung um. l zeigt entlang der positiven Stromrichtung."; break;
  }
  case solenoidB: add(r,"H",b*c/f,"A/m"); add(r,"B",mu0*a*b*c/f,"T"); break;
  case solenoidI: add(r,"I",a*f/(mu0*b*c),"A"); break;
  case solenoidN: case solenoidMu: {
    const double x=a*f/(mu0*b*c); if (!(x>0)) return bad(e);
    add(r,op==solenoidN?"N":"mur",x); break;
  }
  case solenoidLength: {
    const double x=mu0*b*c*f/a;
    if (!(x>0)) return bad(e);
    add(r,"l",x,"m"); break;
  }
  case longwireB: add(r,"B",mu0*a*b/(2*pi*c),"T"); r.note="Unendlich langer gerader Leiter; B vorzeichenbehaftet nach rechter Hand um +Leiterachse."; break;
  case longwireI: add(r,"I",2*pi*c*a/(mu0*b),"A"); break;
  case longwireR: {
    const double x=mu0*b*c/(2*pi*a); if (!(x>0)) return bad(e); add(r,"r",x,"m"); break;
  }
  case parallelF: {
    const double per=-mu0*v[4]*a*b/(2*pi*c);
    add(r,"F/l",per,"N/m"); add(r,"F radial",per*f,"N");
    r.note=per<0?"Gleichgerichtete Stroeme: Anziehung.":per>0?"Gegenlaeufige Stroeme: Abstossung.":"Keine Kraft."; break;
  }
  case parallelI: add(r,"I1",-2*pi*c*a/(mu0*v[4]*b*f),"A"); r.note="Negatives radiales F bedeutet Anziehung und gleichgerichtete Stroeme."; break;
  case wireR: add(r,"A Draht",area(c),"m^2"); add(r,"R",a*b/area(c),"Ohm"); break;
  case wireLengthR: add(r,"Drahtlaenge",a*area(c)/b,"m"); break;
  case coilWire: case winding: {
    double n=a,length=b,coreArea=area(c),wireDiameter=f,rho=v[4],mur=v[5],u=v[6],wireLengthValue=pi*c*a;
    if (op==winding) {
      if (f!=std::floor(f) || f>40) return bad(e,"Lagenzahl muss ganzzahlig zwischen 1 und 40 liegen.");
      const double per=std::floor(a/c+1e-10);
      if (!(per>=1)) return bad(e,"Draht ist dicker als die Spule lang ist.");
      n=per*f; length=a; coreArea=area(b); wireDiameter=c;
      wireLengthValue=pi*per*(f*b+c*f*f);
    }
    const double resistance=rho*wireLengthValue/area(wireDiameter),current=u/resistance;
    add(r,"Drahtlaenge",wireLengthValue,"m"); add(r,"R",resistance,"Ohm");
    coil(r,n,length,coreArea,mur,current);
    r.note="Lange Zylinderspule, konstantes mur (keine Saettigung/Hysterese), ohmscher Draht mit editierbarem rho; keine Zuleitung/Erwaermung. N gegeben: D ist mittlerer Windungsdurchmesser; dicht: volle Windungen, Lagenradien beruecksichtigt."; break;
  }
  case windingH: {
    if (b!=std::floor(b) || b>40) return bad(e,"Lagenzahl muss ganzzahlig zwischen 1 und 40 liegen.");
    add(r,"H",b*c/a,"A/m"); add(r,"B",mu0*f*b*c/a,"T");
    r.note="Dichte lange Wicklung ohne Randfelder: kontinuierliche Windungsdichte Lagen/d, konstantes mur ohne Saettigung."; break;
  }
  case ring: case ringWinding: case ringWindingI: case ringWindingArea: case ringCurrent: {
    double n=a,ri=b,ro=c,current=f,mur=v[4];
    const bool dense=op==ringWinding || op==ringWindingI || op==ringWindingArea;
    const double diameter=op==ringWindingArea?std::sqrt(4*c/pi):c;
    if (dense) { ri=a; ro=b; n=std::floor(2*pi*ri/diameter); }
    if (!(ro>ri)) return bad(e,"Aussenradius muss groesser als Innenradius sein.");
    const double path=pi*(ri+ro),cross=area(ro-ri);
    if (op==ringCurrent) { mur=f; current=v[4]*path/(mu0*mur*n*cross); }
    if (dense && !(n>=1)) return bad(e,"Keine volle Windung am Innenumfang moeglich.");
    if (dense && op!=ringWindingI) {
      const double lw=pi*(ro-ri+diameter)*n,resistance=f*lw/area(diameter);
      current=v[5]/resistance; add(r,"Drahtlaenge",lw,"m"); add(r,"R",resistance,"Ohm");
      add(r,"Uind mittel",mu0*mur*n*n*cross/path*current/v[6],"V");
    }
    add(r,"A Kern",cross,"m^2"); add(r,"lm",path,"m"); coil(r,n,path,cross,mur,current);
    r.note="Ring mit kreisfoermigem Querschnitt; mittlerer magnetischer Weg pi(ri+ro), konstantes mur ohne Saettigung/Hysterese. Dichte Wicklung: ganze Windungen am Innenumfang, Drahtdicke im Drahtweg; rho editierbar, ohne Zuleitung/Erwaermung. Uind beim linearen Ausschalten auf null, kein offener RL-Schalter simuliert."; break;
  }
  case coilNL: add(r,"N",std::sqrt(a*f/(mu0*c*b))); break;
  case coilNH: {
    const double n=a*v[4]/(mu0*c*b*f),length=n*v[4]/f;
    if (!(n>0 && length>0)) return bad(e);
    add(r,"N",n); add(r,"l",length,"m"); break;
  }
  case coilLinkage: { const double n=a*b/c; if (!(n>0)) return bad(e); add(r,"N",n); break; }
  case orbit: case orbitB: case orbitSpeed: {
    const double qm=std::abs(b)/c,speed=op==orbitSpeed?a:std::sqrt(2*qm*a);
    const double magnetic=op==orbitB?speed/(qm*f):f;
    if (!orbitResult(r,speed,qm,magnetic,e)) return false;
    add(r,"B",magnetic,"T"); add(r,"Uacc",speed*speed/(2*qm),"V");
    add(r,"W",0.5*c*speed*speed,"J"); break;
  }
  case orbitU: {
    const double qm=std::abs(c)/f,speed=qm*b*a;
    if (!orbitResult(r,speed,qm,b,e)) return false;
    add(r,"Uacc",speed*speed/(2*qm),"V"); break;
  }
  case orbitQM: {
    const double qm=2*a/((b*c)*(b*c)); if (!orbitResult(r,2*a/(b*c),qm,b,e)) return false; break;
  }
  case particleV: {
    const double speed=std::sqrt(2*std::abs(b)*a/c);
    if (!classical(speed,r,e)) return false;
    add(r,"v",speed,"m/s"); break;
  }
  case particleU: if (!classical(a,r,e)) return false; add(r,"Uacc",c*a*a/(2*std::abs(b)),"V"); break;
  case particleBT: add(r,"B",2*pi*c/(std::abs(b)*a),"T"); break;
  case particleBF: add(r,"B",2*pi*c*a/std::abs(b),"T"); break;
  case particleQMT: add(r,"q/m",2*pi/(a*b),"C/kg"); break;
  case particleVR: {
    const double speed=std::abs(c)*b*a/f;
    if (!classical(speed,r,e)) return false;
    add(r,"v",speed,"m/s"); break;
  }
  case particleM: case particleQ: {
    const double qm=2*a/((c*f)*(c*f)); if (!orbitResult(r,2*a/(c*f),qm,c,e)) return false;
    add(r,op==particleM?"m":"q Betrag",op==particleM?std::abs(b)/qm:b*qm,op==particleM?"kg":"C"); break;
  }
  case transverse: case transverseAcc: {
    const double vx=op==transverse?c:std::sqrt(2*std::abs(a)*c/b),gap=v[4],length=v[5],screen=v[6];
    const double acceleration=a*f/(b*gap),through=length/vx;
    const double hitTime=acceleration==0?through+1:std::sqrt(gap/std::abs(acceleration));
    const bool hit=acceleration!=0 && hitTime<=through;
    const double t=hit?hitTime:through,y=0.5*acceleration*t*t,vy=acceleration*t;
    if (!classical(std::hypot(vx,vy),r,e)) return false;
    add(r,"Plattentreffer",hit?1:0); add(r,"t Feld",t,"s"); add(r,"x Feld",vx*t,"m");
    add(r,"y Feld",y,"m"); add(r,"vy",vy,"m/s"); add(r,"Winkel",std::atan2(vy,vx),"rad");
    add(r,"vx",vx,"m/s"); add(r,"v",std::hypot(vx,vy),"m/s");
    add(r,"ay",acceleration,"m/s^2"); add(r,"Fy",a*f/gap,"N");
    Plot p{"Teilchenbahn","x","m","y","m",{},{},true};
    for (unsigned i=0;i<=32;++i) { const double dt=t*(static_cast<double>(i)/32); p.points.push_back({vx*dt,0.5*acceleration*dt*dt}); }
    if (hit) r.note+=" Plattentreffer vor Austritt: kein Schirmwert berechnet.";
    else { add(r,"y Schirm",y+vy*screen/vx,"m"); add(r,"t Schirm",through+screen/vx,"s"); p.points.push_back({length+screen,y+vy*screen/vx}); }
    r.plots.push_back(std::move(p)); r.note+=" Homogenes Querfeld, Start mittig mit vy0=0, danach feldfreier Drift; E zeigt bei U>0 nach +y."; break;
  }
  case wienV: case wienE: case wienB: {
    const double speed=op==wienV?a/b:op==wienE?a:b;
    if (!classical(speed,r,e)) return false;
    add(r,op==wienV?"v":op==wienE?"E":"B",op==wienE?a*b:a/b,op==wienV?"m/s":op==wienE?"V/m":"T");
    r.note+=" Gekreuzte Felder, Kraefte entgegengesetzt; v senkrecht zu E und B."; break;
  }
  case massspec: case massspecR: {
    const double speed=op==massspec?a/(b*c):c/(f*v[4]);
    const double q=std::abs(op==massspec?v[5]:b),magnetic=op==massspec?f:v[5];
    const double mass=op==massspec?q*magnetic*v[4]/speed:a,radius=mass*speed/(q*magnetic);
    if (!orbitResult(r,speed,q/mass,magnetic,e)) return false;
    add(r,"m",mass,"kg"); add(r,"Abstand Halbkreis",2*radius,"m");
    r.note+=" Idealer Wien-Filter, getrenntes homogenes Analysatorfeld, Detektor nach Halbkreis."; break;
  }
  case cyclotron: case cyclotronTurns: {
    const double q=std::abs(a),turns=op==cyclotron?v[4]:q*c*c*v[4]*v[4]/(4*b*f);
    if (op==cyclotron && (turns!=std::floor(turns) || turns>1e9)) return bad(e,"Umlaufzahl muss ganzzahlig und hoechstens 1e9 sein.");
    const double speed=op==cyclotron?std::sqrt(4*turns*q*f/b):q*c*v[4]/b;
    if (!orbitResult(r,speed,q/b,c,e)) return false;
    add(r,"Umlaeufe",turns); add(r,"Spaltpassagen",op==cyclotron?2*turns:std::ceil(2*turns));
    add(r,"W pro Umlauf",2*q*f,"J"); add(r,"W",0.5*b*speed*speed,"J");
    add(r,"Beschleunigungszeit",turns*2*pi*b/(q*c),"s");
    r.note+=" Start aus Ruhe; zwei Beschleunigungen mit |q| Uspalt pro voller Runde. Zielradius: Umlaeufe kontinuierlich idealisiert, Spaltpassagen aufgerundet; keine relativistische Synchronisation."; break;
  }
  case millikanR: case millikanM: {
    if (!(b>c)) return bad(e,"Tropfendichte muss groesser als Luftdichte sein.");
    const double volume=op==millikanR?4*pi*a*a*a/3:a/b,mass=volume*b,effective=volume*(b-c);
    add(r,"m",mass,"kg"); add(r,"r",op==millikanR?a:std::cbrt(3*volume/(4*pi)),"m");
    add(r,"FG effektiv",effective*v[5],"N"); add(r,"q",effective*v[5]*f/v[4],"C");
    add(r,"q/e",effective*v[5]*f/(v[4]*charge));
    r.note="Schwebender kugelfoermiger Tropfen im homogenen Feld; U/d nach oben orientiert, q vorzeichenbehaftet. Auftrieb optional, keine Stokes-Messreihenauswertung."; break;
  }
  case flux: add(r,"Phi",a*b*cosine(c),"Wb"); r.note="Winkel zur Flaechennormalen, nicht zur Flaeche; positiver Umlaufsinn nach rechter Hand zur Normalen."; break;
  case fluxB: case fluxA: {
    const double divisor=b*cosine(c); if (divisor==0) return bad(e,"Bei 90 Grad ist die Umkehrung nicht eindeutig.");
    const double x=a/divisor; if (op==fluxA && !(x>0)) return bad(e);
    add(r,op==fluxB?"B":"A",x,op==fluxB?"T":"m^2"); break;
  }
  case induction: case inductionStates: {
    const double p0=op==induction?b:b*c*cosine(f),p1=op==induction?c:v[4]*v[5]*cosine(v[6]);
    const double dt=op==induction?f:v[7];
    add(r,"deltaPhi",p1-p0,"Wb"); add(r,"Uind mittel",-a*(p1-p0)/dt,"V");
    r.note="Phi pro Windung; gleiche orientierte Normale fuer beide Zustaende. Mittlere Spannung nach Faraday/Lenz, kein Momentanwert."; break;
  }
  case moving:
    if(norm(a,b,c)>=light)return bad(e,"Leitergeschwindigkeit muss kleiner als c sein.");
    add(r,"U",(b*v[5]-c*v[4])*v[6]+(c*f-a*v[5])*v[7]+(a*v[4]-b*f)*v[8],"V"); r.note="Homogenes B und konstante v entlang des geraden Leiters; orientierte EMK in Richtung des l-Vektors, rechtshaendiges xyz."; break;
  case self: add(r,"Uind mittel",-a*(c-b)/f,"V"); break;
  case selfL: {
    if (c==b) return bad(e,"Ohne Stromaenderung keine eindeutige Induktivitaet.");
    const double l=-a*f/(c-b); if (!(l>0)) return bad(e,"Induktionsspannung muss der Stromaenderung entgegenwirken."); add(r,"L",l,"H"); break;
  }
  case coilL: add(r,"L",mu0*a*b*b*c/f,"H"); break;
  case rlOn: case rlOff: case rlOffInitial: {
    const double resistance=op==rlOff?c:b,initial=op==rlOn?f:op==rlOff?f/b:c;
    const double source=op==rlOn?c/b:0,t=op==rlOffInitial?f:v[4],tau=a/resistance;
    const double decay=std::exp(-t/tau),current=relaxation(initial,source,t/tau);
    const double ul=resistance*(source-initial)*decay;
    add(r,"tau",tau,"s"); add(r,"I0",initial,"A"); add(r,"I",current,"A");
    add(r,"UR",resistance*current,"V"); add(r,"UL",ul,"V"); add(r,"Uind",-ul,"V"); add(r,"W",0.5*a*current*current,"J");
    if (op!=rlOn) add(r,"lostEnergy",0.5*a*initial*initial*(-std::expm1(-2*t/tau)),"J");
    circuitPlot(r,initial,source,tau,t,true);
    r.note="Ideale lineare Induktivitaet. R umfasst gesamten aktiven Kreis. Ausschalten nur im geschlossenen Entladekreis (eigener Raus), kein offener Schalter. UL=L dI/dt (passiv), Uind=-UL."; break;
  }
  case rlTimeOn: return targetTime(f,c/b,v[4],a/b,r,e);
  case rlTimeOff: return targetTime(c,0,f,a/b,r,e);
  case trafoN2: add(r,"N2",c*b/a); break;
  case trafoN1: add(r,"N1",c*a/b); break;
  case trafoU1: add(r,"U1",a*b/c,"V"); break;
  case trafoIdle: case trafoLoad: case trafoEta: case trafoFromV: {
    const double u2=op==trafoFromV?b:a*c/b,n2=op==trafoFromV?c*b/a:c;
    const double i2=op==trafoIdle?0:f,eta=op==trafoEta||op==trafoFromV?v[4]:1;
    const double p2=u2*i2,p1=p2/eta;
    add(r,"U2",u2,"V"); add(r,"N2",n2); add(r,"I2",i2,"A"); add(r,"I1",p1/a,"A");
    add(r,"P2",p2,"W"); add(r,"P1",p1,"W"); add(r,"Verlustleistung",p1-p2,"W"); add(r,"eta",eta);
    r.note="Effektivwerte, ohmsche Last (cos phi=1). Spannungsverhaeltnis stets Windungsverhaeltnis; eta veraendert nur Leistungs-/Strombilanz, kein Modell fuer Spannungsabfall. Idealer Leerlauf ohne Magnetisierungsstrom/Verluste."; break;
  }
  case trafoEfficiency: {
    const double eta=c*f/(a*b); if (!(eta>0 && eta<=1)) return bad(e,"Abgegebene Leistung darf die aufgenommene Leistung nicht uebersteigen."); add(r,"eta",eta); break;
  }
  case trafoI2: {
    const double u2=a*c/b;
    add(r,"U2",u2,"V"); add(r,"I2",v[4]*a*f/u2,"A");
    r.note="Effektivwerte, ohmsche Last. U2/U1=N2/N1 unabhaengig von eta; eta beschreibt nur die Leistungsbilanz."; break;
  }
  }
  if (op>=solenoidB && op<=solenoidLength) r.note="Lange Spule, homogenes Innenfeld, lineares konstantes mur; Randfelder und Saettigung vernachlaessigt.";
  if (op==coilL || op==coilNL || op==coilNH || op==coilLinkage) r.note="Ideale lineare lange Spule, konstanter Fluss pro Windung, kein Streufluss, konstantes mur; kontinuierliche Ziel-Windungszahl.";
  return true;
}

bool inDomain(double x,Domain domain) {
  switch(domain) {
  case positive: return x>0;
  case nonnegative: return x>=0;
  case nonzero: return x!=0;
  case angle: return x>=0 && x<=pi;
  case efficiency: return x>0 && x<=1;
  case any: return true;
  }
  return false;
}
#ifndef PHYSIK_DATA_CATALOG
Parameter parameter(const Input& p) { return {p.name,p.unit,std::string(p.unit)=="rad"?"deg":p.unit}; }
#endif
}

std::vector<NativeCategory> fieldsCategories() {
  return {{"E_FIELD","Elektrische Felder"},{"E_CAP","Kondensatoren"},{"E_RC","RC-Schaltungen"},
          {"E_MAG","Magnetfelder und Spulen"},{"E_PARTICLE","Teilchen in Feldern"},
          {"E_IND","Induktion"},{"E_RL","RL-Schaltungen"},{"E_TRAFO","Transformator"}};
}

#ifndef PHYSIK_DATA_CATALOG
std::vector<Workflow> fieldsCatalog(const std::string& category) {
  std::vector<Workflow> result;
  for (const auto& d:definitions) {
    if (category!=d.category) continue;
    Workflow w;
    w.calculation={d.title,d.formula,"Ergebnisse","",d.id,{}};
    for (const auto& p:d.inputs) { w.calculation.parameters.push_back(parameter(p)); w.defaults.push_back(p.initial); }
    w.minCount=d.min; w.maxCount=d.max;
    for (const auto& p:d.repeated) { w.repeated.push_back(parameter(p)); w.repeatedDefaults.push_back(p.initial); }
    result.push_back(std::move(w));
  }
  return result;
}
#endif

bool fieldsCalculate(const std::string& id,const std::vector<double>& values,NativeResult& result,std::string& error) {
  result={}; error.clear();
  const Definition* found=nullptr;
  for (const auto& d:definitions) if (id==d.id) { found=&d; break; }
  if (!found) return bad(error,"Unbekannte Feld-Rechenmaske.");
  const auto& d=*found;
  for (double x:values) if (!std::isfinite(x)) return bad(error,"Alle Eingaben muessen endliche Zahlen sein.");
#ifdef PHYSIK_DATA_CATALOG
  size_t expected=d.arity;
#else
  size_t expected=d.inputs.size();
#endif
  if (d.max) {
    if (values.empty() || values[0]<d.min || values[0]>d.max || values[0]!=std::floor(values[0]))
      return bad(error,"Anzahl muss ganzzahlig und innerhalb der angegebenen Grenzen liegen.");
#ifdef PHYSIK_DATA_CATALOG
    expected+=static_cast<size_t>(values[0])*d.stride;
#else
    expected+=static_cast<size_t>(values[0])*d.repeated.size();
#endif
  }
  if (values.size()!=expected) return bad(error,"Falsche Anzahl von Eingabewerten.");
#ifdef PHYSIK_DATA_CATALOG
  for(size_t i=0;i<values.size();++i){
    const size_t parameter=i<d.arity?i:d.arity+(i-d.arity)%d.stride;
    const auto domain=static_cast<Domain>((d.domains>>(3*parameter))&7);
    if(!inDomain(values[i],domain)){error="Ungueltiger Wertebereich: Eingabe "+std::to_string(i+1);return false;}
  }
#else
  size_t i=0;
  for (const auto& p:d.inputs) if (!inDomain(values[i++],static_cast<Domain>(p.domain))) {
    error=std::string("Ungueltiger Wertebereich: ")+p.name; return false;
  }
  while (i<values.size()) for (const auto& p:d.repeated) if (!inDomain(values[i++],static_cast<Domain>(p.domain))) {
    error=std::string("Ungueltiger Wertebereich: ")+p.name; return false;
  }
#endif
  NativeResult candidate;
  if (!evaluate(d,values,candidate,error)) return false;
  if (candidate.values.empty()) return bad(error,"Keine endlichen Ergebnisse.");
  for (const auto& value:candidate.values) if (!std::isfinite(value.value))
    return bad(error,"Ergebnis ausserhalb des endlichen Zahlenbereichs; Eingaben pruefen.");
  for (const auto& plot:candidate.plots) {
    if (plot.points.size()>400 || plot.model.size()>400) return bad(error,"Zu viele Diagrammpunkte.");
    for (const auto& point:plot.points) if (!std::isfinite(point.first)||!std::isfinite(point.second)) return bad(error,"Diagramm ausserhalb des Zahlenbereichs.");
    for (const auto& point:plot.model) if (!std::isfinite(point.first)||!std::isfinite(point.second)) return bad(error,"Diagramm ausserhalb des Zahlenbereichs.");
  }
  result=std::move(candidate); return true;
}
}

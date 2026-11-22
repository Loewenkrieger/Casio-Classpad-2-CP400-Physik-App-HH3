// SPDX-License-Identifier: GPL-3.0-or-later
#include "extensions.hpp"
#include <algorithm>
#include <cmath>
#include <initializer_list>

namespace physik {
namespace {
enum Kind {
  Free,Vertical,Horizontal,Oblique,Target,ProjectileState,ProjectileHeight,
  BrakeA,BrakeMu,SafeSpeed,RequiredBrake,Encounter,Passing,
  Components,VectorSum,River,RiverSteer,
  WorkAngle,WorkLift,WorkFriction,WorkSpring,EnergyV,EnergyH,EnergyLoss,
  Elastic,Inelastic,Incline,InclineHeight,
  CircleSpeed,CircleOmega,CircleFrequency,CirclePeriod,CircleRadius,CircleForce,
  FlatCurve,BankedCurve,Carousel,Loop,KindCount
};
#ifdef PHYSIK_DATA_CATALOG
struct Spec {
  const char* id;unsigned arity;
  constexpr Spec(const char* key,const char*,const char*,const char*,unsigned n):id(key),arity(n){}
};
#else
struct Spec { const char *id,*category,*title,*formula; unsigned arity; };
#endif
const Spec specs[]={
  {"M_FREE","M_WURF","Freier Fall","y=h-g*t^2/2",2},
  {"M_VERTICAL","M_WURF","Senkrechter Wurf","y=h+v0*t-g*t^2/2",3},
  {"M_HORIZONTAL","M_WURF","Waagerechter Wurf","x=v0*t; y=h-g*t^2/2",3},
  {"M_OBLIQUE","M_WURF","Schraeger Wurf","x=v0*cos(alpha)*t; y=h+v0*sin(alpha)*t-g*t^2/2",4},
  {"M_TARGET_V","M_WURF","Wurf: Starttempo zum Ziel","v0=x/cos(alpha)*sqrt(g/(2*(x*tan(alpha)-Delta_h)))",5},
  {"M_PROJECTILE_STATE","M_WURF","Wurfzustand zu exakt t","x=vx0*t; y=h0+vy0*t-g*t^2/2; 0<=t<=t_Ziel_absteigend",6},
  {"M_PROJECTILE_HEIGHT","M_WURF","Wurf auf beliebige Zielhoehe","t_Ziel=(vy0 +/- sqrt(vy0^2+2*g*(h0-hZ)))/g",5},
  {"M_BRAKE_A","M_STRASSE","Reaktion und Bremsen (a)","s=v*tR+v^2/(2*a)",4},
  {"M_BRAKE_MU","M_STRASSE","Reaktion und Bremsen (mu)","a=mu*g; s=v*tR+v^2/(2*a)",5},
  {"M_SAFE_V","M_STRASSE","Sichere Anfangsgeschwindigkeit","d=v*tR+v^2/(2*mu*g)",4},
  {"M_REQUIRED_A","M_STRASSE","Erforderliche Verzoegerung","a=v^2/(2*(d-v*tR))",4},
  {"M_ENCOUNTER","M_TREFFEN","Treffen: bis 2 Phasen je Koerper","xA(t)=xB(t); nur innerhalb beider Phasen",5},
  {"M_PASSING","M_TREFFEN","Ueberholen mit Laengen und Luecken","xA-xB=LA+Luecke_nach; A startet hinter B",7},
  {"M_COMPONENTS","M_VEKTOREN","Geschwindigkeit zerlegen","vx=v*cos(alpha); vy=v*sin(alpha)",2},
  {"M_VECTOR_SUM","M_VEKTOREN","Geschwindigkeiten addieren","vx=sum(v*cos(alpha)); vy=sum(v*sin(alpha))",1},
  {"M_RIVER","M_VEKTOREN","Flussfahrt mit Steuerwinkel","vx=u+v*sin(alpha); vy=v*cos(alpha)",4},
  {"M_RIVER_STEER","M_VEKTOREN","Fluss: genau gegenueber anlegen","sin(alpha)=-u/v; t=b/sqrt(v^2-u^2)",3},
  {"M_WORK_ANGLE","M_ENERGIE","Arbeit mit Winkel und Leistung","W=F*s*cos(alpha); P=W/t",4},
  {"M_WORK_LIFT","M_ENERGIE","Hubarbeit und Leistung","W=m*g*Delta_h; P=W/t",4},
  {"M_WORK_FRICTION","M_ENERGIE","Reibungsarbeit und Waerme","W=-F_R*s; Q=F_R*s",2},
  {"M_WORK_SPRING","M_ENERGIE","Federarbeit zwischen 2 Zustaenden","E=k*x^2/2; W_Feder=E1-E2",3},
  {"M_ENERGY_V","M_ENERGIE","Energiebilanz: Endtempo","E2=E1+W_ext-Q",7},
  {"M_ENERGY_H","M_ENERGIE","Energiebilanz: Endhoehe","E2=E1+W_ext-Q",7},
  {"M_ENERGY_LOSS","M_ENERGIE","Energiebilanz: Waermeverlust","Q=E1+W_ext-E2",7},
  {"M_ELASTIC","M_STOSS","Elastischer Stoss 1D","p1=p2; K1=K2",4},
  {"M_INELASTIC","M_STOSS","Vollstaendig unelastischer Stoss 1D","v'=(m1*v1+m2*v2)/(m1+m2)",4},
  {"M_INCLINE","M_EBENE","Schiefe Ebene: Winkel und Weg","F_H=m*g*sin(alpha); N=m*g*cos(alpha)",7},
  {"M_INCLINE_H","M_EBENE","Schiefe Ebene: Hoehe und Laenge","sin(alpha)=h/L; F_R=mu*N",7},
  {"M_CIRCLE_V","M_KREIS","Kreis aus Bahngeschwindigkeit","omega=v/r; F=m*v^2/r",3},
  {"M_CIRCLE_OMEGA","M_KREIS","Kreis aus Winkelgeschwindigkeit","v=omega*r",3},
  {"M_CIRCLE_F","M_KREIS","Kreis aus Frequenz","omega=2*pi*f",3},
  {"M_CIRCLE_T","M_KREIS","Kreis aus Umlaufzeit","omega=2*pi/T",3},
  {"M_CIRCLE_R","M_KREIS","Kreisradius aus v und omega","r=v/omega",3},
  {"M_CIRCLE_FORCE_R","M_KREIS","Kreisradius aus Kraft","r=m*v^2/F",3},
  {"M_FLAT_CURVE","M_KREIS","Ebene Kurve: Haftgrenze","v_max=sqrt(mu*g*r)",3},
  {"M_BANKED_CURVE","M_KREIS","Ueberhoehte Kurve: Tempobereich","v^2=r*g*(sin(alpha)+/-mu*cos(alpha))/(cos(alpha)-/+mu*sin(alpha))",4},
  {"M_CAROUSEL","M_KREIS","Kegelpendel und Kettenkarussell","r=R+L*sin(alpha); omega^2=g*tan(alpha)/r",5},
  {"M_LOOP","M_KREIS","Innenlooping ohne Zwangsfuehrung","N=m*(v^2/r+g*cos(phi)); h_min=5*r/2-v0^2/(2*g)",6}
};
static_assert(sizeof(specs)/sizeof(specs[0])==KindCount,"Mechanikkatalog");

#ifndef PHYSIK_DATA_CATALOG
Parameter p(const char* name,const char* unit,const char* def=nullptr){
  return {name,unit,def?def:unit};
}
void inputs(Workflow& w,std::initializer_list<Parameter> parameters,
            std::initializer_list<const char*> defaults){
  w.calculation.parameters=parameters;
  for(const char* value:defaults)w.defaults.emplace_back(value);
}
Workflow workflow(Kind k){
  const Spec& s=specs[k];
  Workflow w;
  w.calculation={s.title,s.formula,"Ergebnisse","",s.id,{}};
  switch(k){
    case Free:inputs(w,{p("Starthoehe","m"),p("g","m/s^2")},{"2.5","9.81"});break;
    case Vertical:case Horizontal:
      inputs(w,{p("Starthoehe","m"),p(k==Vertical?"v0 (aufwaerts positiv)":"v0","m/s"),p("g","m/s^2")},
             {"2.5","4","9.81"});break;
    case Oblique:
      inputs(w,{p("Starthoehe","m"),p("v0","m/s","km/h"),p("Abwurfwinkel","rad","deg"),p("g","m/s^2")},
             {"0","100","30","9.81"});break;
    case Target:
      inputs(w,{p("Zielentfernung x","m"),p("Starthoehe","m"),p("Zielhoehe","m"),
                p("Abwurfwinkel","rad","deg"),p("g","m/s^2")},{"6","2.5","0","0","9.81"});break;
    case ProjectileState:case ProjectileHeight:
      inputs(w,{p("Starthoehe (Bezugsniveau frei)","m"),p("Zielhoehe (Bezugsniveau frei)","m"),p("v0","m/s","km/h"),
                p("Abwurfwinkel","rad","deg"),p("g","m/s^2")},{"0","0","100","30","9.81"});
      if(k==ProjectileState){w.calculation.parameters.push_back(p("Exakte Zeit ab Start","s"));w.defaults.push_back("1");}break;
    case BrakeA:
      inputs(w,{p("v0","m/s","km/h"),p("Reaktionszeit","s"),p("Verzoegerungsbetrag","m/s^2"),p("Hindernisabstand","m")},
             {"80","1","4.905","9"});break;
    case BrakeMu:
      inputs(w,{p("v0","m/s","km/h"),p("Reaktionszeit","s"),p("Reibungszahl mu","1"),p("g","m/s^2"),p("Hindernisabstand","m")},
             {"80","1","0.5","9.81","9"});break;
    case SafeSpeed:
      inputs(w,{p("Verfuegbarer Weg","m"),p("Reaktionszeit","s"),p("Reibungszahl mu","1"),p("g","m/s^2")},
             {"72.561382","1","0.5","9.81"});break;
    case RequiredBrake:
      inputs(w,{p("v0","m/s","km/h"),p("Reaktionszeit","s"),p("Verfuegbarer Weg","m"),p("g","m/s^2")},
             {"80","1","72.561382","9.81"});break;
    case Encounter:case Passing:
      if(k==Encounter)inputs(w,{p("Phasen je Koerper","1"),p("xA0","m"),p("vA0","m/s"),p("xB0","m"),p("vB0","m/s")},
                             {"1","0","10","100","0"});
      else inputs(w,{p("Phasen je Koerper","1"),p("vA0","m/s","km/h"),p("vB0","m/s","km/h"),
                     p("Laenge A","m"),p("Laenge B","m"),p("Luecke vorher","m"),p("Luecke nachher","m")},
                    {"1","100","60","4.5","15","40","40"});
      w.minCount=1;w.maxCount=2;
      w.repeated={p("Dauer A (2. darf 0 sein)","s"),p("aA","m/s^2"),p("Dauer B (2. darf 0 sein)","s"),p("aB","m/s^2")};
      w.repeatedDefaults={"30","0","30","0"};break;
    case Components:
      inputs(w,{p("Betrag v","m/s"),p("Winkel ab +x","rad","deg")},{"5","30"});break;
    case VectorSum:
      inputs(w,{p("Anzahl Vektoren","1")},{"2"});w.minCount=1;w.maxCount=40;
      w.repeated={p("Betrag v","m/s"),p("Winkel ab +x","rad","deg")};w.repeatedDefaults={"4","0"};break;
    case River:case RiverSteer:
      inputs(w,{p("Flussbreite","m"),p("Stroemung (+x)","m/s"),p("Boot relativ Wasser","m/s")},{"90","1.5","4"});
      if(k==River){w.calculation.parameters.push_back(p("Steuerwinkel ab quer (+ zur Stroemung)","rad","deg"));w.defaults.push_back("0");}break;
    case WorkAngle:
      inputs(w,{p("Kraftbetrag","N"),p("Weg","m"),p("Kraftwinkel zum Weg","rad","deg"),p("Zeit","s")},
             {"500","200","30","300"});break;
    case WorkLift:
      inputs(w,{p("Masse","kg"),p("Hoehenaenderung","m"),p("g","m/s^2"),p("Zeit","s")},{"60","8","9.81","10"});break;
    case WorkFriction:inputs(w,{p("Reibungskraftbetrag","N"),p("Weg","m")},{"50","25"});break;
    case WorkSpring:inputs(w,{p("Federkonstante","N/m"),p("Auslenkung 1","m"),p("Auslenkung 2","m")},{"100","0.2","0"});break;
    case EnergyV:
      inputs(w,{p("Masse","kg"),p("v1 Betrag","m/s"),p("h1","m"),p("h2","m"),p("Aeussere Arbeit","J"),p("Waerme Q","J"),p("g","m/s^2")},
             {"60","0","8","2","0","1250","9.81"});break;
    case EnergyH:case EnergyLoss:
      inputs(w,{p("Masse","kg"),p("v1 Betrag","m/s"),p("h1","m"),p("v2 Betrag","m/s"),
                p(k==EnergyH?"Aeussere Arbeit":"h2",k==EnergyH?"J":"m"),
                p(k==EnergyH?"Waerme Q":"Aeussere Arbeit","J"),p("g","m/s^2")},
             k==EnergyH?std::initializer_list<const char*>{"60","0","8","0","0","1250","9.81"}:
                        std::initializer_list<const char*>{"60","0","8","0","2","0","9.81"});break;
    case Elastic:case Inelastic:
      inputs(w,{p("Masse 1","kg","g"),p("Masse 2","kg","g"),p("v1 signiert","m/s"),p("v2 signiert","m/s")},
             {"40","60","4","0"});break;
    case Incline:
      inputs(w,{p("Masse","kg"),p("Neigung","rad","deg"),p("Haftzahl","1"),p("Gleitreibzahl","1"),
                p("v0 (+ abwaerts)","m/s"),p("Weg in Bewegungsrichtung","m"),p("g","m/s^2")},
             {"80","30","0.1","0.05","0","200","9.81"});break;
    case InclineHeight:
      inputs(w,{p("Masse","kg"),p("Hoehe","m"),p("Laenge","m"),p("Haftzahl","1"),p("Gleitreibzahl","1"),
                p("v0 (+ abwaerts)","m/s"),p("g","m/s^2")},{"80","100","200","0.1","0.05","0","9.81"});break;
    case CircleSpeed:case CircleOmega:case CircleFrequency:case CirclePeriod:
      inputs(w,{p("Radius (nicht Durchmesser)","m"),
                p(k==CircleSpeed?"Bahngeschwindigkeit":k==CircleOmega?"Winkelgeschwindigkeit":k==CircleFrequency?"Frequenz":"Umlaufzeit",
                  k==CircleSpeed?"m/s":k==CircleOmega?"rad/s":k==CircleFrequency?"Hz":"s",k==CircleSpeed?"km/h":nullptr),p("Masse","kg")},
             {"10",k==CircleSpeed?"30":k==CircleOmega?"0.8333333333333333":k==CircleFrequency?"0.1326291192432461":"7.539822368615504","60"});break;
    case CircleRadius:
      inputs(w,{p("Bahngeschwindigkeit","m/s"),p("Winkelgeschwindigkeit","rad/s"),p("Masse","kg")},{"8","0.8","60"});break;
    case CircleForce:
      inputs(w,{p("Masse","kg"),p("Bahngeschwindigkeit","m/s"),p("Zentripetalkraft","N")},{"60","8","384"});break;
    case FlatCurve:
      inputs(w,{p("Radius","m"),p("Haftzahl","1"),p("g","m/s^2")},{"50","0.5","9.81"});break;
    case BankedCurve:
      inputs(w,{p("Radius","m"),p("Ueberhoehung","rad","deg"),p("Haftzahl","1"),p("g","m/s^2")},
             {"50","15","0.2","9.81"});break;
    case Carousel:
      inputs(w,{p("Aufhaengungsradius R","m"),p("Fadenlaenge L","m"),p("Winkel zur Senkrechten","rad","deg"),
                p("Masse","kg"),p("g","m/s^2")},{"0","2","30","60","9.81"});break;
    case Loop:
      inputs(w,{p("Masse","kg"),p("Radius","m"),p("Starthoehe ab Tiefpunkt","m"),p("Starttempo","m/s"),
                p("Bahnwinkel ab Tiefpunkt","rad","deg"),p("g","m/s^2")},{"60","5","12.5","0","180","9.81"});break;
    case KindCount:break;
  }
  return w;
}
#endif
bool fail(std::string& error,const char* message){error=message;return false;}
void add(NativeResult& r,const char* name,double value,const char* unit){r.values.push_back({name,value,unit});}
bool validResult(const NativeResult& r){
  if(r.values.empty())return false;
  for(const auto& v:r.values)if(!std::isfinite(v.value))return false;
  for(const auto& plot:r.plots){
    if(plot.points.size()>400||plot.model.size()>400)return false;
    for(const auto& point:plot.points)if(!std::isfinite(point.first)||!std::isfinite(point.second))return false;
    for(const auto& point:plot.model)if(!std::isfinite(point.first)||!std::isfinite(point.second))return false;
  }
  return true;
}
void vectorResult(NativeResult& r,double x,double y){
  add(r,"vx",x,"m/s");add(r,"vy",y,"m/s");add(r,"v",std::hypot(x,y),"m/s");
  add(r,"alpha",x==0&&y==0?0:std::atan2(y,x),"rad");
  if(x==0&&y==0)r.note+=" Nullvektor: Richtung nicht definiert, alpha=0 als Konvention.";
}
bool projectile(NativeResult& r,double h,double vx,double vy,double g,std::string& error){
  if(h<0||g<=0)return fail(error,"Starthoehe muss >= 0 und g > 0 sein.");
  const double d=std::hypot(vy,std::sqrt(2*g*h));
  const double t=vy>=0?(vy+d)/g:(h==0?0:2*h/(d-vy));
  const double peak=std::max(0.0,vy)/g,hitVy=vy-g*t;
  add(r,"v0",std::hypot(vx,vy),"m/s");add(r,"vx0",vx,"m/s");add(r,"vy0",vy,"m/s");
  add(r,"t_Flug",t,"s");add(r,"Reichweite",vx*t,"m");add(r,"t_Scheitel",peak,"s");
  add(r,"h_max",h+std::max(0.0,vy)*peak/2,"m");
  add(r,"vx_Aufprall",vx,"m/s");add(r,"vy_Aufprall",hitVy,"m/s");
  add(r,"v_Aufprall",std::hypot(vx,hitVy),"m/s");
  add(r,"alpha_Aufprall",vx==0&&hitVy==0?0:std::atan2(hitVy,vx),"rad");
  Plot plot{"Wurfbahn","x","m","y","m",{},{},true};
  for(unsigned i=0;i<=64;++i){
    const double ti=t*(i/64.0);
    plot.points.emplace_back(vx*ti,i==64?0:h+(vy-g*ti/2)*ti);
  }
  r.plots.push_back(std::move(plot));
  r.note="Ohne Luftwiderstand; +y aufwaerts; Landung auf y=0; g ist der Betrag.";
  return true;
}
bool projectileHeight(NativeResult& r,const std::vector<double>& z,bool state,std::string& error){
  const double h0=z[0],target=z[1],v0=z[2],angle=z[3],g=z[4],delta=h0-target;
  if(v0<0||std::abs(angle)>pi/2||g<=0)
    return fail(error,"Starttempo muss >= 0, g > 0 und -90 <= Winkel <= 90 Grad sein.");
  if(!std::isfinite(delta))return fail(error,"Hoehendifferenz liegt ausserhalb des Zahlenbereichs.");
  const double vx=std::abs(angle)==pi/2?0:v0*std::cos(angle),vy=v0*std::sin(angle);
  if(delta<0&&vy<=0)return fail(error,"Hoeheres Ziel ist ohne aufwaerts gerichtete Startgeschwindigkeit unerreichbar.");
  // Work in velocity magnitudes, avoiding squared velocities in the discriminant.
  const double change=std::sqrt(g)*std::sqrt(std::abs(delta))*std::sqrt(2.0);
  if(!std::isfinite(change)||(delta!=0&&change==0))return fail(error,"Zielhoehengleichung ist nicht darstellbar.");
  double d=0;
  if(delta>=0)d=std::hypot(vy,change);
  else {
    const double ratio=change/vy;
    if(ratio>1+8*2.220446049250313e-16)return fail(error,"Zielhoehe liegt oberhalb des Scheitels und ist unerreichbar.");
    const double bounded=std::min(1.0,ratio);
    d=vy*std::sqrt((1-bounded)*(1+bounded));
  }
  const double sum=d+std::abs(vy);
  const double end=vy>=0?sum/g:(sum==0?0:2*delta/sum);
  const bool upHit=vy>=0&&delta<=0;
  double ascending=d==0?end:vy>=0?(sum==0?0:-2*delta/sum):-sum/g;
  if(ascending==0)ascending=0;
  if(!std::isfinite(end)||!std::isfinite(ascending)||end<0||(end==0&&delta!=0)||(upHit&&delta<0&&ascending==0))
    return fail(error,"Kein darstellbarer absteigender Zielhoehentreffer ab dem Start.");
  if(state&&(z[5]<0||z[5]>end))return fail(error,"Zeit muss zwischen 0 und dem absteigenden Zielhoehentreffer liegen.");
  const double peak=std::max(0.0,vy)/g;
  add(r,"v0",v0,"m/s");add(r,"vx0",vx,"m/s");add(r,"vy0",vy,"m/s");
  add(r,"h_Start",h0,"m");add(r,"h_Ziel",target,"m");add(r,"t_Scheitel",peak,"s");
  add(r,"h_max",h0+std::max(0.0,vy)*peak/2,"m");
  add(r,"t_Flug",end,"s");add(r,"Reichweite",vx*end,"m");
  add(r,"t_Ziel_absteigend",end,"s");add(r,"Aufsteigender_Treffer",upHit?1:0,"1");
  if(upHit)add(r,"t_Ziel_aufsteigend",ascending,"s");
  add(r,"Zieltreffer",upHit&&d>0?2:1,"1");
  add(r,"vx_Ziel",vx,"m/s");add(r,"vy_Ziel",-d,"m/s");add(r,"v_Ziel",std::hypot(vx,d),"m/s");
  add(r,"alpha_Ziel",vx==0&&d==0?0:std::atan2(-d,vx),"rad");
  r.note="Ohne Luftwiderstand; x0=0, +y aufwaerts; Hoehennullpunkt frei. Flugende ist der absteigende Zielhoehentreffer, nicht y=0.";
  if(!upHit)r.note+=" Aufsteigender Hoehentreffer liegt vor dem Start und gehoert nicht zum Flug.";
  if(d==0)r.note+=" Scheitelberuehrung: auf- und absteigende Hoehenzeit fallen zusammen.";
  if(state){
    const double t=z[5],y=t==end?target:std::fma(vy-g*t/2,t,h0),stateVy=t==end?-d:vy-g*t;
    add(r,"t",t,"s");add(r,"x",vx*t,"m");add(r,"y",y,"m");vectorResult(r,vx,stateVy);
  }
  Plot plot{"Flug bis absteigendem Zieltreffer","x","m","y","m",{},{},true};
  for(unsigned i=0;i<=64;++i){
    const double t=end*(i/64.0);
    plot.points.emplace_back(vx*t,i==64?target:std::fma(vy-g*t/2,t,h0));
  }
  r.plots.push_back(std::move(plot));return true;
}
bool braking(NativeResult& r,double v,double tr,double a,double obstacle,std::string& error){
  if(v<0||tr<0||a<=0||obstacle<0)return fail(error,"v, Reaktionszeit und Abstand muessen >= 0, Bremsbetrag > 0 sein.");
  const double reaction=v*tr,brake=v*v/(2*a),stop=reaction+brake;
  add(r,"v0",v,"m/s");add(r,"a_Betrag",a,"m/s^2");add(r,"s_Reaktion",reaction,"m");
  add(r,"s_Bremsen",brake,"m");add(r,"s_Anhalten",stop,"m");
  add(r,"t_Bremsen",v/a,"s");add(r,"t_Anhalten",v==0?0:tr+v/a,"s");
  int phase=0;
  if(v>0&&obstacle<stop){
    phase=tr>0&&obstacle<=reaction?1:2;
    const double hit=phase==1?v:std::sqrt(std::max(0.0,v*v-2*a*(obstacle-reaction)));
    const double time=phase==1?obstacle/v:tr+2*(obstacle-reaction)/(v+hit);
    add(r,"t_Hindernis",time,"s");add(r,"v_Hindernis",hit,"m/s");
  }
  add(r,"Hindernisphase",phase,"1");
  r.note="Ebene Fahrbahn, konstante Verzoegerung nach Reaktionszeit. Phase 0: Stillstand vor/am Hindernis; 1: Reaktion; 2: Bremsen.";
  return true;
}

struct Phase {double start,duration,x,v,a;};
struct Body {std::vector<Phase> phases;double end=0;};
bool body(Body& b,double x,double v,unsigned n,const std::vector<double>& z,unsigned offset,unsigned component,
          bool forward,std::string& error){
  for(unsigned i=0;i<n;++i){
    const double dt=z[offset+4*i+component],a=z[offset+4*i+component+1];
    if(dt<0||(i==0&&dt==0))return fail(error,"Erste Phasendauer muss > 0 sein; zweite darf 0 sein.");
    if(dt==0)continue;
    b.phases.push_back({b.end,dt,x,v,a});
    x+=(v+a*dt/2)*dt;v+=a*dt;b.end+=dt;
    if(!std::isfinite(x)||!std::isfinite(v)||!std::isfinite(b.end))return fail(error,"Phasenwerte ueberschreiten den Zahlenbereich.");
    if(forward&&v<0)return fail(error,"Ueberholen: kein Richtungswechsel innerhalb der eingegebenen Phasen.");
  }
  return true;
}
const Phase& at(const Body& b,double t){
  return b.phases.size()==2&&t>=b.phases[1].start?b.phases[1]:b.phases[0];
}
double position(const Body& b,double t){const auto& q=at(b,t);const double dt=t-q.start;return q.x+(q.v+q.a*dt/2)*dt;}
double velocity(const Body& b,double t){const auto& q=at(b,t);return q.v+q.a*(t-q.start);}

// Scale the polynomial before solving, then keep only roots in this finite interval.
bool roots(double a,double b,double c,double duration,std::vector<double>& out,std::string& error){
  if(!std::isfinite(a)||!std::isfinite(b)||!std::isfinite(c))return fail(error,"Bewegungsgleichung ausserhalb des Zahlenbereichs.");
  const double scale=std::max({std::abs(a),std::abs(b),std::abs(c)});
  if(scale==0)return fail(error,"Nicht eindeutig: Koerper stimmen waehrend eines ganzen Zeitabschnitts ueberein.");
  const double originalA=a,originalB=b,originalC=c;
  a/=scale;b/=scale;c/=scale;
  auto keep=[&](double t){
    if(!std::isfinite(t)||t<0||t>duration)return;
    const double quadratic=(originalA*t)*t,linear=originalB*t;
    const double residual=std::fma(std::fma(originalA,t,originalB),t,originalC);
    const double magnitude=std::max({std::abs(quadratic),std::abs(linear),std::abs(originalC)});
    if(std::isfinite(residual)&&std::isfinite(magnitude)&&std::abs(residual)<=64*2.220446049250313e-16*magnitude)
      out.push_back(t);
  };
  if(a==0){if(b!=0)keep(-c/b);return true;}
  double discriminant=std::fma(b,b,-4*a*c);
  const double tolerance=16*2.220446049250313e-16*(b*b+std::abs(4*a*c));
  if(discriminant< -tolerance)return true;
  discriminant=std::max(0.0,discriminant);
  const double q=-0.5*(b+std::copysign(std::sqrt(discriminant),b));
  if(q==0)keep(-b/(2*a));
  else {keep(q/a);if(discriminant!=0)keep(c/q);}
  return true;
}
bool encounter(NativeResult& r,const std::vector<double>& z,bool passing,std::string& error){
  const unsigned n=static_cast<unsigned>(z[0]),base=passing?7:5;
  Body a,b;
  double target=0;
  if(passing){
    if(z[1]<0||z[2]<0||z[3]<=0||z[4]<=0||z[5]<0||z[6]<0)
      return fail(error,"Ueberholen: Anfangstempi >= 0, Laengen > 0, Luecken >= 0 erforderlich.");
    target=z[3]+z[6];
    if(!body(a,0,z[1],n,z,base,0,true,error)||!body(b,z[4]+z[5],z[2],n,z,base,2,true,error))return false;
    add(r,"Relativweg",z[3]+z[4]+z[5]+z[6],"m");
  }else if(!body(a,z[1],z[2],n,z,base,0,false,error)||!body(b,z[3],z[4],n,z,base,2,false,error))return false;
  const double end=std::min(a.end,b.end);
  std::vector<double> boundaries{0,end},times;
  for(const Body* obj:{&a,&b})for(const auto& q:obj->phases)if(q.start>0&&q.start<end)boundaries.push_back(q.start);
  std::sort(boundaries.begin(),boundaries.end());
  boundaries.erase(std::unique(boundaries.begin(),boundaries.end()),boundaries.end());
  for(std::size_t i=1;i<boundaries.size();++i){
    const double start=boundaries[i-1],duration=boundaries[i]-start;
    std::vector<double> local;
    if(!roots((at(a,start).a-at(b,start).a)/2,velocity(a,start)-velocity(b,start),
              position(a,start)-position(b,start)-target,duration,local,error))return false;
    for(double t:local){
      t+=start;
      if(!passing||velocity(a,t)>=velocity(b,t))times.push_back(t);
    }
  }
  std::sort(times.begin(),times.end());
  times.erase(std::unique(times.begin(),times.end()),times.end());
  if(times.empty())return fail(error,"Kein gueltiges Treffen bzw. vollendetes Ueberholen innerhalb beider Bewegungsabschnitte.");
  add(r,"Loesungen",static_cast<double>(times.size()),"1");add(r,"t",times.front(),"s");add(r,"Gueltig_bis",end,"s");
  for(std::size_t i=0;i<times.size();++i){
    const std::string suffix="_"+std::to_string(i+1);
    const double t=times[i];
    add(r,("t"+suffix).c_str(),t,"s");add(r,("xA"+suffix).c_str(),position(a,t),"m");
    add(r,("xB"+suffix).c_str(),position(b,t),"m");
    add(r,("vA"+suffix).c_str(),velocity(a,t),"m/s");add(r,("vB"+suffix).c_str(),velocity(b,t),"m/s");
  }
  for(const Body* obj:{&a,&b}){
    Plot plot{obj==&a?"Bewegung A":"Bewegung B","t","s","x","m",{},{},true};
    for(unsigned i=0;i<=64;++i){const double t=end*(i/64.0);plot.points.emplace_back(t,position(*obj,t));}
    r.plots.push_back(std::move(plot));
  }
  r.note="Signierte Orte und Geschwindigkeiten; je Koerper 1-2 endliche, stetig verbundene Phasen mit konstantem a; keine Extrapolation.";
  if(passing)r.note+=" A/B sind Fahrzeugfronten; erste Loesung ist das vollendete Ueberholen inklusive beider Laengen und Luecken.";
  return true;
}
void circle(NativeResult& r,double radius,double omega,double mass){
  const double v=radius*omega,a=v*omega;
  add(r,"r",radius,"m");add(r,"v",v,"m/s");add(r,"omega",omega,"rad/s");
  add(r,"f",omega/(2*pi),"Hz");add(r,"T",2*pi/omega,"s");
  add(r,"a_z",a,"m/s^2");add(r,"F_z",mass*a,"N");
}

bool solve(Kind k,const std::vector<double>& z,NativeResult& r,std::string& error){
  switch(k){
    case Free:return projectile(r,z[0],0,0,z[1],error);
    case Vertical:return projectile(r,z[0],0,z[1],z[2],error);
    case Horizontal:
      if(z[1]<0)return fail(error,"Starttempo muss >= 0 sein.");
      return projectile(r,z[0],z[1],0,z[2],error);
    case Oblique:{
      if(z[1]<0||std::abs(z[2])>pi/2)return fail(error,"v0 muss >= 0, Winkel zwischen -90 und 90 Grad sein.");
      const double vx=std::abs(z[2])==pi/2?0:z[1]*std::cos(z[2]);
      return projectile(r,z[0],vx,z[1]*std::sin(z[2]),z[3],error);
    }
    case Target:{
      if(z[0]<=0||z[1]<0||z[2]<0||std::abs(z[3])>=pi/2||z[4]<=0)
        return fail(error,"Zielentfernung und g > 0, Hoehen >= 0 und -90 < Winkel < 90 Grad erforderlich.");
      const double drop=z[0]*std::tan(z[3])-(z[2]-z[1]);
      if(drop<=0)return fail(error,"Ziel liegt auf/oberhalb der Starttangente: kein endliches Starttempo.");
      const double vx=z[0]*std::sqrt(z[4]/(2*drop)),vy=vx*std::tan(z[3]),t=z[0]/vx;
      if(!projectile(r,z[1],vx,vy,z[4],error))return false;
      add(r,"t_Ziel",t,"s");add(r,"y_Ziel",z[2],"m");add(r,"vy_Ziel",vy-z[4]*t,"m/s");
      add(r,"v_Ziel",std::hypot(vx,vy-z[4]*t),"m/s");return true;
    }
    case ProjectileState:case ProjectileHeight:return projectileHeight(r,z,k==ProjectileState,error);
    case BrakeA:return braking(r,z[0],z[1],z[2],z[3],error);
    case BrakeMu:
      if(z[2]<=0||z[3]<=0)return fail(error,"Reibungszahl und g muessen > 0 sein.");
      return braking(r,z[0],z[1],z[2]*z[3],z[4],error);
    case SafeSpeed:{
      if(z[0]<0||z[1]<0||z[2]<=0||z[3]<=0)return fail(error,"Weg und Reaktionszeit >= 0, Reibungszahl und g > 0 erforderlich.");
      const double a=z[2]*z[3];
      const double v=z[0]==0?0:2*z[0]/(z[1]+std::hypot(z[1],std::sqrt(2*z[0]/a)));
      if(!braking(r,v,z[1],a,z[0],error))return false;
      add(r,"v_sicher",v,"m/s");return true;
    }
    case RequiredBrake:{
      if(z[0]<=0||z[1]<0||z[2]<=z[0]*z[1]||z[3]<=0)
        return fail(error,"v und g > 0, Reaktionszeit >= 0; Weg muss groesser als der Reaktionsweg sein.");
      const double a=z[0]*z[0]/(2*(z[2]-z[0]*z[1]));
      if(!braking(r,z[0],z[1],a,z[2],error))return false;
      add(r,"mu_erforderlich",a/z[3],"1");return true;
    }
    case Encounter:case Passing:return encounter(r,z,k==Passing,error);
    case Components:
      if(z[0]<0)return fail(error,"Geschwindigkeitsbetrag muss >= 0 sein.");
      vectorResult(r,z[0]*std::cos(z[1]),z[0]*std::sin(z[1]));return true;
    case VectorSum:{
      double x=0,y=0;
      Plot plot{"Vektorkette","vx","m/s","vy","m/s",{{0,0}},{},true};
      for(unsigned i=0;i<static_cast<unsigned>(z[0]);++i){
        if(z[1+2*i]<0)return fail(error,"Vektorbetrag muss >= 0 sein.");
        x+=z[1+2*i]*std::cos(z[2+2*i]);y+=z[1+2*i]*std::sin(z[2+2*i]);plot.points.emplace_back(x,y);
      }
      vectorResult(r,x,y);r.plots.push_back(std::move(plot));return true;
    }
    case River:case RiverSteer:{
      if(z[0]<=0||z[2]<=0)return fail(error,"Flussbreite und Bootstempo muessen > 0 sein.");
      if(k==RiverSteer&&std::abs(z[1])>=z[2])return fail(error,"Gegenueber anlegen erfordert Bootstempo > Betrag der Stroemung.");
      const double angle=k==River?z[3]:std::asin(-z[1]/z[2]);
      if(std::abs(angle)>=pi/2)return fail(error,"Steuerwinkel muss zwischen -90 und 90 Grad liegen.");
      const double x=k==RiverSteer?0:z[1]+z[2]*std::sin(angle),y=z[2]*std::cos(angle),t=z[0]/y;
      vectorResult(r,x,y);add(r,"Steuerwinkel",angle,"rad");add(r,"t_Ueberfahrt",t,"s");add(r,"Abdrift",x*t,"m");
      r.plots.push_back({"Flussfahrt","stromabwaerts","m","quer","m",{{0,0},{x*t,z[0]}},{},true});
      r.note="Stroemung +x, gegenueberliegendes Ufer +y. Steuerwinkel ab +y, positiv stromabwaerts; konstante Geschwindigkeiten.";return true;
    }
    case WorkAngle:{
      if(z[0]<0||z[1]<0||z[3]<=0)return fail(error,"Kraftbetrag und Weg >= 0, Zeit > 0 erforderlich.");
      const double force=z[0]*std::cos(z[2]),work=force*z[1];
      add(r,"F_parallel",force,"N");add(r,"W",work,"J");add(r,"P",work/z[3],"W");return true;
    }
    case WorkLift:{
      if(z[0]<=0||z[2]<=0||z[3]<=0)return fail(error,"Masse, g und Zeit muessen > 0 sein.");
      const double force=z[0]*z[2],work=force*z[1];
      add(r,"F_G",force,"N");add(r,"W",work,"J");add(r,"P",work/z[3],"W");r.note="Quasistatisch; negative Hoehenaenderung bedeutet negative Hubarbeit.";return true;
    }
    case WorkFriction:
      if(z[0]<0||z[1]<0)return fail(error,"Reibungskraftbetrag und Weg muessen >= 0 sein.");
      add(r,"W_Reibung",-z[0]*z[1],"J");add(r,"Q",z[0]*z[1],"J");return true;
    case WorkSpring:
      if(z[0]<=0)return fail(error,"Federkonstante muss > 0 sein.");
      add(r,"E1",z[0]*z[1]*z[1]/2,"J");add(r,"E2",z[0]*z[2]*z[2]/2,"J");
      add(r,"W_Feder",z[0]*(z[1]*z[1]-z[2]*z[2])/2,"J");
      add(r,"W_aussen",-r.values.back().value,"J");r.note="Ideale Hooke-Feder, signierte Auslenkung ab Ruhelage; aeussere Arbeit quasistatisch.";return true;
    case EnergyV:case EnergyH:case EnergyLoss:{
      const double m=z[0],v1=z[1],h1=z[2],g=z[6];
      if(m<=0||v1<0||g<=0||(k!=EnergyV&&z[3]<0)||(k!=EnergyLoss&&z[5]<0))
        return fail(error,"Masse und g > 0, Geschwindigkeitsbetraege und Waerme >= 0 erforderlich.");
      double v2=k==EnergyV?0:z[3],h2=k==EnergyH?0:z[k==EnergyV?3:4];
      const double work=z[k==EnergyV||k==EnergyH?4:5];
      double heat=k==EnergyLoss?0:z[5];
      const double e1=m*(v1*v1/2+g*h1);
      if(k==EnergyV){
        const double square=v1*v1+2*g*(h1-h2)+2*(work-heat)/m;
        if(square<0)return fail(error,"Energie reicht fuer die vorgegebene Endhoehe nicht aus.");
        v2=std::sqrt(square);
      }else if(k==EnergyH)h2=h1+(v1*v1-v2*v2)/(2*g)+(work-heat)/(m*g);
      const double e2=m*(v2*v2/2+g*h2);
      if(k==EnergyLoss){
        heat=e1+work-e2;
        if(heat<0&&-heat<=16*2.220446049250313e-16*std::max({std::abs(e1),std::abs(work),std::abs(e2)}))heat=0;
        if(heat<0)return fail(error,"Negative Verlustwaerme: Zustaende erfordern zusaetzliche Energiezufuhr.");
      }
      add(r,"v2",v2,"m/s");add(r,"h2",h2,"m");add(r,"Q",heat,"J");
      add(r,"W_ext",work,"J");add(r,"E1",e1,"J");add(r,"E2",e2,"J");
      add(r,"h_max_ideal",h2+v2*v2/(2*g),"m");
      r.note="E=m*v^2/2+m*g*h; E2=E1+W_ext-Q. Hoehen ab frei waehlbarem Nullniveau. h_max_ideal bei anschliessender verlustfreier Umwandlung aller Bewegungsenergie.";return true;
    }
    case Elastic:case Inelastic:{
      if(z[0]<=0||z[1]<=0)return fail(error,"Beide Massen muessen > 0 sein.");
      const double scale=std::max(z[0],z[1]),m1=z[0]/scale,m2=z[1]/scale;
      const double fraction=m1/(m1+m2),other=m2/(m1+m2),common=fraction*z[2]+other*z[3];
      const double relative=z[2]-z[3];
      const double v1=k==Inelastic?common:common-other*relative;
      const double v2=k==Inelastic?common:common+fraction*relative;
      const double before=(z[0]*z[2]*z[2]+z[1]*z[3]*z[3])/2,after=(z[0]*v1*v1+z[1]*v2*v2)/2;
      add(r,"v1_nach",v1,"m/s");add(r,"v2_nach",v2,"m/s");add(r,"p_vor",z[0]*z[2]+z[1]*z[3],"kg*m/s");
      add(r,"p_nach",z[0]*v1+z[1]*v2,"kg*m/s");add(r,"K_vor",before,"J");add(r,"K_nach",after,"J");add(r,"Delta_K",after-before,"J");
      r.note="Isolierter, zentraler Stoss in 1D, beide Geschwindigkeiten auf derselben signierten Achse; tatsaechlicher Kontakt vorausgesetzt.";return true;
    }
    case Incline:case InclineHeight:{
      const double m=z[0],g=z[6],ms=z[k==Incline?2:3],mk=z[k==Incline?3:4],v0=z[k==Incline?4:5];
      if(m<=0||g<=0||mk<0||ms<mk)return fail(error,"Masse und g > 0; Haftzahl >= Gleitreibzahl >= 0 erforderlich.");
      if(k==InclineHeight&&(z[2]<=0||z[1]<0||z[1]>=z[2]))return fail(error,"Laenge > 0 und 0 <= Hoehe < Laenge erforderlich.");
      const double angle=k==Incline?z[1]:std::asin(z[1]/z[2]),distance=z[k==Incline?5:2];
      if(angle<0||angle>=pi/2||distance<0)return fail(error,"0 <= Neigung < 90 Grad und Weg >= 0 erforderlich.");
      const double down=m*g*std::sin(angle),normal=m*g*std::cos(angle);
      const bool stationary=v0==0&&std::tan(angle)<=ms;
      const double direction=v0<0?-1:1,friction=stationary?-down:-direction*mk*normal,a=(down+friction)/m;
      add(r,"alpha",angle,"rad");add(r,"F_H",down,"N");add(r,"N",normal,"N");add(r,"F_R",friction,"N");
      add(r,"F_res",down+friction,"N");add(r,"a",a,"m/s^2");add(r,"Haftet",stationary?1:0,"1");
      r.note="+ entlang Ebene abwaerts. Haftreibung nur bei v0=0; sonst Gleitreibung entgegen Bewegung. Auswertung endet spaetestens beim ersten Stillstand.";
      if(stationary){add(r,"Weg_erreicht",distance==0?1:0,"1");add(r,"v_aktuell",0,"m/s");if(distance==0)add(r,"t",0,"s");else r.note+=" Vorgegebener Weg wird aus Ruhe nicht erreicht.";return true;}
      const double along=a*direction,speed=std::abs(v0),square=speed*speed+2*along*distance;
      if(along<0){add(r,"s_Stillstand",speed*speed/(-2*along),"m");add(r,"t_Stillstand",speed/(-along),"s");}
      add(r,"Weg_erreicht",square<0?0:1,"1");
      if(square<0){r.note+=" Vorgegebener Weg liegt hinter dem ersten Stillstand; keine Fortsetzung mit unveraenderter Reibungsrichtung.";return true;}
      const double endSpeed=std::sqrt(square);
      add(r,"v_Ende",direction*endSpeed,"m/s");
      add(r,"t",distance==0?0:2*distance/(speed+endSpeed),"s");return true;
    }
    case CircleSpeed:case CircleOmega:case CircleFrequency:case CirclePeriod:case CircleRadius:case CircleForce:{
      if(z[0]<=0||z[1]<=0||z[2]<=0)return fail(error,"Alle Kreisgroessen muessen als positive Betraege eingegeben werden.");
      double radius=z[0],omega=z[1],mass=z[2];
      if(k==CircleSpeed)omega=z[1]/radius;
      else if(k==CircleFrequency)omega=2*pi*z[1];
      else if(k==CirclePeriod)omega=2*pi/z[1];
      else if(k==CircleRadius)radius=z[0]/omega;
      else if(k==CircleForce){mass=z[0];radius=mass*z[1]*z[1]/z[2];omega=z[1]/radius;}
      if(radius<=0||omega<=0)return fail(error,"Radius oder Winkelgeschwindigkeit ist nicht darstellbar.");
      circle(r,radius,omega,mass);return true;
    }
    case FlatCurve:
      if(z[0]<=0||z[1]<0||z[2]<=0)return fail(error,"Radius und g > 0, Haftzahl >= 0 erforderlich.");
      add(r,"v_min",0,"m/s");add(r,"v_max",std::sqrt(z[1]*z[2]*z[0]),"m/s");add(r,"a_max",z[1]*z[2],"m/s^2");
      r.note="Ebene Kurve; seitliche Haftreibung; keine gleichzeitige Brems-/Antriebskraft.";return true;
    case BankedCurve:{
      if(z[0]<=0||z[1]<0||z[1]>=pi/2||z[2]<0||z[3]<=0)return fail(error,"Radius und g > 0; Haftzahl >= 0; 0 <= Ueberhoehung < 90 Grad erforderlich.");
      const double s=std::sin(z[1]),c=std::cos(z[1]),mu=z[2],rg=z[0]*z[3],denom=c-mu*s;
      add(r,"v_min",std::sqrt(rg*std::max(0.0,s-mu*c)/(c+mu*s)),"m/s");
      add(r,"v_ohne_Reibung",std::sqrt(rg*s/c),"m/s");add(r,"Endliche_Obergrenze",denom>0?1:0,"1");
      if(denom>0)add(r,"v_max",std::sqrt(rg*(s+mu*c)/denom),"m/s");
      r.note=denom>0?"Stationaere Kurvenfahrt; Haftreibung verhindert Rutschen nach innen/unten bzw. aussen/oben.":
        "Ideales Haftmodell hat keine endliche obere Tempogrenze; kein unendlicher Zahlenwert ausgegeben.";return true;
    }
    case Carousel:{
      if(z[0]<0||z[1]<=0||z[2]<=0||z[2]>=pi/2||z[3]<=0||z[4]<=0)
        return fail(error,"R >= 0; Laenge, Masse, g > 0; 0 < Fadenwinkel < 90 Grad erforderlich.");
      const double radius=z[0]+z[1]*std::sin(z[2]),omega=std::sqrt(z[4]*std::tan(z[2])/radius);
      circle(r,radius,omega,z[3]);add(r,"F_Seil",z[3]*z[4]/std::cos(z[2]),"N");
      r.note="Kegelpendel: R=0; Karussell: R ist Radius der Aufhaengung. Winkel zur Senkrechten, stationaer und ohne Luftwiderstand.";return true;
    }
    case Loop:{
      const double mass=z[0],radius=z[1],height=z[2],v0=z[3],phi=z[4],g=z[5];
      if(mass<=0||radius<=0||height<0||v0<0||phi<0||phi>2*pi||g<=0)
        return fail(error,"Masse, Radius, g > 0; Starthoehe und Starttempo >= 0; 0 <= Bahnwinkel <= 360 Grad erforderlich.");
      const double bottom=v0*v0+2*g*height,q=bottom/(g*radius);
      if(!std::isfinite(q)||g*radius==0)return fail(error,"Loopingenergie liegt ausserhalb des darstellbaren Zahlenbereichs.");
      const bool full=q>=5;
      const double limit=full?2*pi:std::acos(q<2?1-q/2:(2-q)/3);
      add(r,"h_min_Start",std::max(0.0,2.5*radius-v0*v0/(2*g)),"m");
      add(r,"v_min_unten",std::sqrt(5*g*radius),"m/s");add(r,"v_min_oben",std::sqrt(g*radius),"m/s");
      add(r,"Voller_Loop",full?1:0,"1");add(r,"phi_Grenze",limit,"rad");
      add(r,"Kontakt_am_Ziel",phi<=limit?1:0,"1");
      if(phi<=limit){
        const double c=std::cos(phi),square=std::max(0.0,bottom-2*g*radius*(1-c));
        add(r,"v",std::sqrt(square),"m/s");add(r,"N",std::max(0.0,mass*(square/radius+g*c)),"N");
        add(r,"h",radius*(1-c),"m");
      }
      Plot plot{"Erste zusammenhaengende Kontaktfahrt","phi","rad","v","m/s",{},{},true};
      for(unsigned i=0;i<=64;++i){const double angle=limit*(i/64.0);plot.points.emplace_back(angle,std::sqrt(std::max(0.0,bottom-2*g*radius*(1-std::cos(angle)))));}
      r.plots.push_back(std::move(plot));
      r.note="Reibungsfreie Innenbahn, keine Zwangsfuehrung: N >= 0. Startenergie m*g*h0+m*v0^2/2; Fahrt vom Tiefpunkt aufwaerts.";
      if(!full)r.note+=q<2?" Energie reicht nur bis zum Umkehrpunkt; phi_Grenze ist dieser Punkt.":
        " Kontaktverlust bei phi_Grenze; danach keine Fortsetzung auf der Kreisbahn.";
      return true;
    }
    case KindCount:break;
  }
  return fail(error,"Unbekannte Mechanikmaske.");
}
}

std::vector<NativeCategory> mechanicsCategories(){
  return {{"M_WURF","Fall und Wurf"},{"M_STRASSE","Reaktion und Bremsen"},{"M_TREFFEN","Treffen und Ueberholen"},
          {"M_VEKTOREN","Bewegung in 2D"},{"M_ENERGIE","Arbeit und Energie"},{"M_STOSS","Stoesse 1D"},
          {"M_EBENE","Schiefe Ebene"},{"M_KREIS","Kreis, Kurven und Looping"}};
}
#ifndef PHYSIK_DATA_CATALOG
std::vector<Workflow> mechanicsCatalog(const std::string& category){
  std::vector<Workflow> result;
  for(unsigned i=0;i<KindCount;++i)if(category==specs[i].category)result.push_back(workflow(static_cast<Kind>(i)));
  return result;
}
#endif
bool mechanicsCalculate(const std::string& id,const std::vector<double>& values,NativeResult& result,std::string& error){
  result={};error.clear();
  unsigned index=0;
  while(index<KindCount&&id!=specs[index].id)++index;
  if(index==KindCount)return fail(error,"Unbekannte Mechanik-ID.");
  for(double v:values)if(!std::isfinite(v))return fail(error,"Alle Eingaben muessen endlich sein.");
  const Kind k=static_cast<Kind>(index);
  std::size_t arity=specs[index].arity;
  if(k==Encounter||k==Passing||k==VectorSum){
    const unsigned maximum=k==VectorSum?40:2;
    if(values.empty()||values[0]<1||values[0]>maximum||std::floor(values[0])!=values[0])
      return fail(error,"Anzahl muss ganzzahlig sein und innerhalb der Maskengrenzen liegen.");
    arity+=static_cast<unsigned>(values[0])*(k==VectorSum?2:4);
  }
  if(values.size()!=arity)return fail(error,"Falsche Anzahl Eingaben fuer diese Mechanikmaske.");
  NativeResult candidate;
  if(!solve(k,values,candidate,error))return false;
  if(!validResult(candidate))return fail(error,"Ergebnis liegt ausserhalb des endlichen Zahlenbereichs.");
  result=std::move(candidate);return true;
}
}

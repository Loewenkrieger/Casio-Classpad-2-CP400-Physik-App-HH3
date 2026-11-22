// SPDX-License-Identifier: GPL-3.0-or-later
#include "extensions.hpp"
#include <algorithm>
#include <cmath>

namespace physik {
namespace {
constexpr unsigned stepLimit=50000;
constexpr double tolerance=.001;
enum Kind { RL,Spring,Fall };
struct State { double x=0,v=0,w=0; };
struct Model { Kind kind; double a,b,c; bool released=false; };
struct Sample { double t; State y; };
struct Run {
  State end,probe;
  double t=0,releaseTime=0,releaseSpeed=0,t99=0,s99=0,peak=0;
  bool stopped=false,hasProbe=false,has99=false,hasRelease=false;
  std::vector<Sample> samples;
};
struct Budget { unsigned steps=0; std::string error; };

bool finite(State y){return std::isfinite(y.x)&&std::isfinite(y.v)&&std::isfinite(y.w);}
bool fail(Budget& b,const char* text){b.error=text;return false;}
State plus(State y,State k,double h){return {y.x+h*k.x,y.v+h*k.v,y.w+h*k.w};}
State derivative(const Model& m,State y){
  if(m.kind==RL)return {m.a-m.b*y.x,0,0};
  if(m.kind==Spring)return {y.v,(m.released?0:m.a*(m.c-y.x))-m.b,m.b*y.v};
  const double drag=m.b*y.v*std::abs(y.v);
  return {y.v,m.a-drag,drag*y.v};
}
bool step(const Model& m,State y,double h,State& next,Budget& b){
  if(b.steps>=stepLimit)return fail(b,"Mehr als 50000 RK4-Schritte; dt erhoehen oder Zeitraum/Intervall verkleinern");
  ++b.steps;
  const State k1=derivative(m,y),y2=plus(y,k1,h/2);
  const State k2=derivative(m,y2),y3=plus(y,k2,h/2);
  const State k3=derivative(m,y3),y4=plus(y,k3,h);
  const State k4=derivative(m,y4);
  next={y.x+h*(k1.x+2*k2.x+2*k3.x+k4.x)/6,
        y.v+h*(k1.v+2*k2.v+2*k3.v+k4.v)/6,
        y.w+h*(k1.w+2*k2.w+2*k3.w+k4.w)/6};
  if(!finite(k1)||!finite(k2)||!finite(k3)||!finite(k4)||!finite(next))
    return fail(b,"Zahlenbereich im RK4-Modell ueberschritten");
  return true;
}

// Reintegrate the smooth phase from its left endpoint, never interpolate an overshoot.
bool root(const Model& m,State left,double h,double target,bool velocity,
          bool rising,State& at,double& elapsed,Budget& b){
  double lo=0,hi=h;
  for(unsigned i=0;i<36;++i){
    const double mid=lo+(hi-lo)/2;
    if(!step(m,left,mid,at,b))return false;
    if(((velocity?at.v:at.x)<target)==rising)lo=mid;else hi=mid;
  }
  elapsed=lo+(hi-lo)/2;
  if(!step(m,left,elapsed,at,b))return false;
  if(velocity)at.v=target;else at.x=target;
  return true;
}
void save(Run& r,double t,State y,bool capture){
  if(!capture)return;
  if(!r.samples.empty()&&r.samples.back().t==t)r.samples.back().y=y;
  else if(r.samples.size()<400)r.samples.push_back({t,y});
}
bool run(Model m,State y,double horizon,double dt,double probe,bool capture,Run& r,Budget& b){
  r={};r.peak=y.v;
  // A successful coarse+fine pair cannot consume more than 16666 coarse steps.
  const unsigned stride=std::max(1u,static_cast<unsigned>(std::ceil(std::min(16000.0,horizon/dt)/340)));
  unsigned tick=0;
  if(m.kind==Spring){
    m.released=y.x>=m.c;
    if(y.v==0&&((m.released?0:m.a*(m.c-y.x))<=m.b))r.stopped=true;
  }
  double terminal=0,criterion=0;
  bool rising=true;
  if(m.kind==Fall){
    if(y.x==probe){r.probe=y;r.hasProbe=true;}
    if(m.b>0){
      terminal=std::sqrt(m.a/m.b);
      rising=y.v<terminal;
      criterion=(rising?.99:1.01)*terminal;
      if(std::abs(y.v-terminal)<=.01*terminal){r.has99=true;r.s99=y.x;}
    }
  }
  save(r,0,y,capture);
  double t=0;
  while(t<horizon&&!r.stopped){
    double remaining=std::min(dt,horizon-t);
    if(!(t+remaining>t))return fail(b,"dt ist im Zahlenbereich zu klein");
    // A spring step has at most two smooth pieces, separated by release.
    do {
      State next;
      double h=remaining;
      if(!step(m,y,h,next,b))return false;
      bool release=false,stop=false;
      if(m.kind==Spring){
        if(!m.released&&next.x>=m.c){
          if(!root(m,y,h,m.c,false,true,next,h,b))return false;
          release=true;
        }
        if(next.v<=0&&(y.v>0||m.a*(m.c-y.x)>m.b)){
          if(!root(m,y,h,0,true,false,next,h,b))return false;
          release=false;stop=true;
        }
        // Locate the speed maximum before changing the force law.
        const double peakX=m.c-m.b/m.a;
        if(!m.released&&y.x<peakX&&next.x>=peakX){
          State peak;double hp=0;
          if(!root(m,y,h,peakX,false,true,peak,hp,b))return false;
          r.peak=std::max(r.peak,peak.v);
        }
      }else if(m.kind==Fall){
        if(next.x>=m.c){
          if(!root(m,y,h,m.c,false,true,next,h,b))return false;
          stop=true;
        }
        if(!r.hasProbe&&y.x<=probe&&next.x>=probe){
          double hp=0;
          if(!root(m,y,h,probe,false,true,r.probe,hp,b))return false;
          r.hasProbe=true;
          save(r,t+hp,r.probe,capture);
        }
        if(m.b>0&&!r.has99&&((rising&&next.v>=criterion)||(!rising&&next.v<=criterion))){
          State at;double hp=0;
          if(!root(m,y,h,criterion,true,rising,at,hp,b))return false;
          r.has99=true;r.t99=t+hp;r.s99=at.x;
        }
      }
      if(m.kind!=RL&&(next.v<0||next.x<y.x||next.w<y.w))
        return fail(b,"Schrittweite ungeeignet: unphysikalische Umkehr; dt verkleinern");
      r.peak=std::max(r.peak,next.v);
      t+=h;y=next;
      if(stop){r.stopped=true;remaining=0;}
      else if(release){
        m.released=true;r.hasRelease=true;r.releaseTime=t;r.releaseSpeed=y.v;
        remaining=std::max(0.0,remaining-h);
        save(r,t,y,capture);
      }else remaining=0;
    }while(remaining>0);
    if(++tick%stride==0)save(r,t,y,capture);
  }
  r.t=t;r.end=y;
  save(r,t,y,capture);
  return true;
}

double difference(double a,double b){
  return std::abs(a-b)/std::max(1e-12,std::max(std::abs(a),std::abs(b)));
}
void compare(double a,double b,double& worst){worst=std::max(worst,difference(a,b));}
bool pairRun(Model m,State initial,double horizon,double dt,double probe,
             Run& coarse,Run& fine,double& worst,Budget& b){
  if(!run(m,initial,horizon,dt,probe,true,coarse,b)||
     !run(m,initial,horizon,dt/2,probe,false,fine,b))return false;
  compare(coarse.end.x,fine.end.x,worst);
  compare(coarse.end.v,fine.end.v,worst);
  compare(coarse.end.w,fine.end.w,worst);
  compare(coarse.t,fine.t,worst);
  compare(coarse.peak,fine.peak,worst);
  if(coarse.stopped!=fine.stopped||coarse.hasProbe!=fine.hasProbe||
     coarse.has99!=fine.has99||coarse.hasRelease!=fine.hasRelease)worst=std::max(worst,1.0);
  if(coarse.hasProbe&&fine.hasProbe){
    compare(coarse.probe.v,fine.probe.v,worst);compare(coarse.probe.w,fine.probe.w,worst);
  }
  if(coarse.has99&&fine.has99){compare(coarse.t99,fine.t99,worst);compare(coarse.s99,fine.s99,worst);}
  if(coarse.hasRelease&&fine.hasRelease){
    compare(coarse.releaseTime,fine.releaseTime,worst);compare(coarse.releaseSpeed,fine.releaseSpeed,worst);
  }
  return true;
}
void value(NativeResult& r,const std::string& name,double x,const std::string& unit){r.values.push_back({name,x,unit});}
Plot plot(const char* title,const char* x,const char* xu,const char* y,const char* yu){
  return {title,x,xu,y,yu,{},{},true};
}
bool positive(double x){return x>0&&std::isfinite(x);}
bool timeDomain(double t,double dt,Budget& b){
  if(!positive(t)||!positive(dt)||dt>t||!(dt/2>0))
    return fail(b,"Zeitraum und dt muessen positiv und darstellbar sein: dt <= t");
  return true;
}
bool modelDomain(const Model& m,State y,double dt,Budget& b){
  double rate=m.b;
  if(m.kind==Spring)rate=std::sqrt(m.a);
  if(m.kind==Fall)rate=std::max(std::sqrt(m.a*m.b),m.b*y.v);
  if(!finite(y)||!std::isfinite(m.a)||!std::isfinite(m.b)||!std::isfinite(rate)||
     !std::isfinite(rate*dt)||rate*dt>(m.kind==RL?2:1))
    return fail(b,"Modellparameter oder dt ausserhalb des stabilen Zahlenbereichs; dt verkleinern");
  if(m.kind==Fall&&m.b>0&&!positive(std::sqrt(m.a/m.b)))
    return fail(b,"Grenzgeschwindigkeit ausserhalb des Zahlenbereichs");
  return true;
}

bool springModel(const std::vector<double>& v,double friction,Model& m,State& y,Budget& b){
  if(!positive(v[0])||!positive(v[1])||v[2]<0||friction<0||v[4]<0||v[5]<0)
    return fail(b,"Wagen: m und D positiv; Spannung, Reibung, s0 und v0 nicht negativ");
  m={Spring,v[1]/v[0],friction/v[0],v[2]};y={v[4],v[5],0};
  if(!(m.a>0)||(friction>0&&!(m.b>0))||
     (v[2]>v[4]&&!(m.a*(v[2]-v[4])>0)))
    return fail(b,"Wagenkoeffizienten ausserhalb des Zahlenbereichs");
  return timeDomain(v[6],v[7],b)&&modelDomain(m,y,v[7],b);
}
bool fallModel(const std::vector<double>& v,Model& m,State& y,Budget& b){
  if(!positive(v[0])||v[1]<0||v[2]<0||v[3]<0||!positive(v[4])||
     v[5]<0||v[5]>=v[4]||v[6]<0||!positive(v[7])||v[10]<v[5]||v[10]>v[4])
    return fail(b,"Fall: m,g,h positiv; A,rho,cW,v0 >= 0; 0 <= s0 < h; Messweg zwischen s0 und h");
  m={Fall,v[7],(.5*v[2]*v[3]*v[1])/v[0],v[4]};y={v[5],v[6],0};
  if(v[1]>0&&v[2]>0&&v[3]>0&&!(m.b>0))
    return fail(b,"Luftwiderstandskoeffizient ausserhalb des Zahlenbereichs");
  return timeDomain(v[8],v[9],b)&&modelDomain(m,y,v[9],b);
}
double rlExact(double u,double resistance,double l,double initial,double t){
  const double decay=std::exp(-(resistance/l)*t);
  return initial*decay+(u/resistance)*(-std::expm1(-(resistance/l)*t));
}

bool rlResult(const std::vector<double>& v,NativeResult& out,double& worst,Budget& b){
  if(!positive(v[1])||!positive(v[2]))return fail(b,"Spule: R und L muessen positiv sein");
  const Model m={RL,v[0]/v[2],v[1]/v[2],0};const State initial={v[3],0,0};
  if(!(m.b>0)||(v[0]!=0&&m.a==0))return fail(b,"Spulenkoeffizienten ausserhalb des Zahlenbereichs");
  if(!timeDomain(v[4],v[5],b)||!modelDomain(m,initial,v[5],b))return false;
  Run a,z;
  if(!pairRun(m,initial,v[4],v[5],0,a,z,worst,b))return false;
  Plot p=plot("Abi 2013: I(t)","t","s","I","A");
  const double steady=v[0]/v[1],tau=v[2]/v[1];
  for(const auto& s:a.samples){
    const double exact=rlExact(v[0],v[1],v[2],v[3],s.t);
    p.points.emplace_back(s.t,s.y.x);p.model.emplace_back(s.t,exact);
    worst=std::max(worst,std::abs(s.y.x-exact)/std::max(1e-12,std::max(std::abs(steady),std::abs(v[3]))));
  }
  out.plots.push_back(std::move(p));
  value(out,"I(t Ende)",a.end.x,"A");value(out,"tau",tau,"s");value(out,"I unendlich",steady,"A");
  value(out,"t99 Restfehler",v[3]==steady?0:tau*std::log(100.0),"s");
  out.note+="RK4; Modellkurve analytisch. t99: Restabstand zum Endstrom <= 1% des Anfangsabstands, nicht exakt stationaer. ";
  return true;
}
bool springResult(const std::vector<double>& v,NativeResult& out,double& worst,Budget& b,const std::string& suffix=""){
  Model m={Spring,0,0,0};State initial;
  if(!springModel(v,v[3],m,initial,b))return false;
  Run a,z;
  if(!pairRun(m,initial,v[6],v[7],0,a,z,worst,b))return false;
  Plot vs=plot("Abi 2014: v(s)","s","m","v","m/s"),st=plot("Wagen: s(t)","t","s","s","m");
  vs.title+=suffix;st.title+=suffix;
  for(const auto& s:a.samples){vs.points.emplace_back(s.y.x,s.y.v);st.points.emplace_back(s.t,s.y.x);}
  out.plots.push_back(std::move(vs));out.plots.push_back(std::move(st));
  value(out,"FR"+suffix,v[3],"N");value(out,"s Ende"+suffix,a.end.x,"m");
  value(out,"t Ende"+suffix,a.t,"s");value(out,"v Ende"+suffix,a.end.v,"m/s");value(out,"v maximal"+suffix,a.peak,"m/s");
  value(out,"Stillstand erreicht"+suffix,a.stopped?1:0,"");
  const double compression=std::max(0.0,v[2]-v[4]),left=std::max(0.0,v[2]-a.end.x);
  const double energy=.5*v[0]*v[5]*v[5]+.5*v[1]*compression*compression;
  const double balance=energy-(.5*v[0]*a.end.v*a.end.v+.5*v[1]*left*left+v[3]*(a.end.x-v[4]));
  value(out,"Energierest"+suffix,balance,"J");
  worst=std::max(worst,std::abs(balance)/std::max(1e-12,energy));
  if(a.hasRelease){value(out,"t Feder frei"+suffix,a.releaseTime,"s");value(out,"v Feder frei"+suffix,a.releaseSpeed,"m/s");}
  if(a.stopped)out.note+="Erster Stillstand lokalisiert; keine kuenstliche Rueckwaertsbewegung. ";
  else out.note+="Zeithorizont erreicht, noch kein Stillstand (bei FR=0 ggf. dauerhaftes Rollen). ";
  if(a.stopped&&a.t==0)out.note+="Anfangskraft <= Reibung: Wagen bleibt in Ruhe. ";
  return true;
}
bool fallResult(const std::vector<double>& v,NativeResult& out,double& worst,Budget& b){
  Model m={Fall,0,0,0};State initial;
  if(!fallModel(v,m,initial,b))return false;
  Run a,z;
  if(!pairRun(m,initial,v[8],v[9],v[10],a,z,worst,b))return false;
  Plot fs=plot("Abi 2016: F netto(s)","s","m","F netto","N"),
       ws=plot("Positive Reibungsarbeit", "s","m","WR","J"),
       st=plot("Fall: s(t)","t","s","s","m");
  for(const auto& s:a.samples){
    fs.points.emplace_back(s.y.x,v[0]*(m.a-m.b*s.y.v*std::abs(s.y.v)));
    ws.points.emplace_back(s.y.x,v[0]*s.y.w);st.points.emplace_back(s.t,s.y.x);
  }
  out.plots.push_back(std::move(fs));out.plots.push_back(std::move(ws));out.plots.push_back(std::move(st));
  value(out,"s Ende",a.end.x,"m");value(out,"t Ende",a.t,"s");value(out,"v Ende",a.end.v,"m/s");
  value(out,"Boden erreicht",a.stopped?1:0,"");if(a.stopped)value(out,"Fallzeit",a.t,"s");
  value(out,"WR Ende",v[0]*a.end.w,"J");
  const double balance=v[0]*(v[7]*(a.end.x-v[5])+.5*(v[6]*v[6]-a.end.v*a.end.v)-a.end.w);
  value(out,"Energierest",balance,"J");
  worst=std::max(worst,std::abs(balance)/std::max(1e-12,v[0]*(v[7]*v[4]+.5*v[6]*v[6])));
  if(a.hasProbe){
    value(out,"Messweg",v[10],"m");value(out,"WR am Messweg",v[0]*a.probe.w,"J");
    value(out,"WR / mgh am Messweg",100*a.probe.w/v[7]/v[4],"%");
  }else out.note+="Messweg im Zeithorizont nicht erreicht. ";
  if(m.b>0){
    value(out,"Grenzgeschwindigkeit",std::sqrt(m.a/m.b),"m/s");
    if(a.has99){value(out,"t99 Geschwindigkeit",a.t99,"s");value(out,"s99 Geschwindigkeit",a.s99,"m");}
    else out.note+="99%-Geschwindigkeitskriterium vor Ende nicht erreicht. ";
  }else out.note+="Ohne Luftwiderstand keine endliche Grenzgeschwindigkeit. ";
  out.note+="Fallrichtung positiv; WR ist dissipierte Arbeit ab Start. 99% bedeutet |v-vGrenz| <= 0.01*vGrenz, F netto ist nicht exakt null. ";
  if(!a.stopped)out.note+="Boden im Zeithorizont nicht erreicht. ";
  return true;
}

// Stopping distance from the same piecewise energy law, including rest before release.
double springStop(double mass,double d,double compression,double s0,double v0,double friction){
  const double c=std::max(0.0,compression-s0),force=d*c;
  if(v0==0&&force<=friction)return s0;
  const double energy=.5*mass*v0*v0+.5*d*c*c;
  if(energy>=friction*c)return s0+energy/friction;
  const double q=force-friction;
  const double disc=std::sqrt(q*q+d*mass*v0*v0);
  const double distance=q>=0?(q+disc)/d:(mass*v0*v0)/(disc-q);
  return s0+distance;
}
template<class Function>
bool bounded(double lo,double hi,double target,bool increasing,unsigned iterations,
             Function f,double& solution,Budget& b){
  double a=0,z=0;
  if(!f(lo,a)||!f(hi,z))return false;
  if(!std::isfinite(a)||!std::isfinite(z)||a==z||
     (increasing?!(a<z):!(a>z))||target<std::min(a,z)||target>std::max(a,z))
    return fail(b,"Messwert nicht eindeutig im monotonen Suchintervall eingeschlossen");
  if(target==a){solution=lo;return true;}if(target==z){solution=hi;return true;}
  for(unsigned i=0;i<iterations;++i){
    solution=lo+(hi-lo)/2;
    double y=0;
    if(!f(solution,y))return false;
    if(!std::isfinite(y))return fail(b,"Suchmodell ausserhalb des Zahlenbereichs");
    if((y<target)==increasing)lo=solution;else hi=solution;
  }
  solution=lo+(hi-lo)/2;return true;
}
bool inverseRL(const std::vector<double>& v,NativeResult& out,double& worst,Budget& b){
  if(!positive(v[0])||!positive(v[1])||v[2]<0||!positive(v[3])||!positive(v[4])||
     !positive(v[5])||!(v[6]>v[5])||v[3]>v[7]||!timeDomain(v[7],v[8],b))
    return fail(b,"R-Suche: U,L,t,I und positive R-Grenzen pruefen; Messzeit <= Zeithorizont");
  double r=0;
  if(!bounded(v[5],v[6],v[4],false,48,[&](double x,double& y){
      y=rlExact(v[0],x,v[1],v[2],v[3]);return true;
    },r,b))return false;
  if(!rlResult({v[0],r,v[1],v[2],v[7],v[8]},out,worst,b))return false;
  Run check;const Model m={RL,v[0]/v[1],r/v[1],0};
  if(!run(m,{v[2],0,0},v[3],v[8],0,false,check,b))return false;
  compare(check.end.x,v[4],worst);
  value(out,"R aus Messwert",r,"Ohm");value(out,"I am Messzeitpunkt",check.end.x,"A");
  out.plots.front().model={{v[3],v[4]}};
  out.note+="R positiv, begrenzte monotone Suche am exakten RL-Modell; Messwert mit RK4 geprueft. ";
  return true;
}
bool inverseSpring(const std::vector<double>& v,NativeResult& out,double& worst,Budget& b){
  if(!positive(v[6])||!(v[7]>v[6])||!(v[5]>v[3]))
    return fail(b,"Reibungssuche: positive geordnete FR-Grenzen und Zielweg > s0 erforderlich");
  const std::vector<double> base={v[0],v[1],v[2],v[6],v[3],v[4],v[8],v[9]};
  Model m={Spring,0,0,0};State y;
  if(!springModel(base,v[6],m,y,b))return false;
  double friction=0;
  if(!bounded(v[6],v[7],v[5],false,48,[&](double x,double& s){
      s=springStop(v[0],v[1],v[2],v[3],v[4],x);return true;
    },friction,b))return false;
  if(!springResult({v[0],v[1],v[2],friction,v[3],v[4],v[8],v[9]},out,worst,b))return false;
  // The public inverse must not claim a stop that lies beyond the horizon.
  if(out.values[5].value!=1)return fail(b,"Zielstillstand liegt ausserhalb des Zeithorizonts");
  compare(out.values[1].value,v[5],worst);
  value(out,"FR aus Zielstillstand",friction,"N");value(out,"Zielstillstand",v[5],"m");
  out.note+="Begrenzte monotone Suche mit Energiebilanz, Stillstand durch RK4 geprueft. ";
  return true;
}
bool inverseFall(const std::vector<double>& v,NativeResult& out,double& worst,Budget& b){
  if(!positive(v[1])||!positive(v[2])||!positive(v[7])||v[8]<0||!(v[9]>v[8])||v[7]>v[10])
    return fail(b,"cW-Suche: A,rho und Messzeit positiv; 0 <= cW min < max; Messzeit <= Zeithorizont");
  std::vector<double> base={v[0],v[1],v[2],v[8],v[3],v[4],v[5],v[6],v[10],v[11],v[12]};
  Model m={Fall,0,0,0};State y;
  if(!fallModel(base,m,y,b))return false;
  double cw=0;
  if(!bounded(v[8],v[9],v[7],true,18,[&](double x,double& t){
      base[3]=x;
      if(!fallModel(base,m,y,b))return false;
      Run trial;
      if(!run(m,y,v[10],v[11],v[12],false,trial,b))return false;
      if(!trial.stopped)return fail(b,"Suchintervall erreicht den Boden nicht im Zeithorizont");
      t=trial.t;return true;
    },cw,b))return false;
  base[3]=cw;
  if(!fallResult(base,out,worst,b))return false;
  // Fallzeit only exists for a localized ground event.
  bool found=false;
  for(const auto& x:out.values)if(x.name=="Fallzeit"){compare(x.value,v[7],worst);found=true;}
  if(!found)return fail(b,"Gefundener cW-Wert erreicht den Boden nicht");
  value(out,"cW aus Fallzeit",cw,"");value(out,"Gemessene Fallzeit",v[7],"s");
  out.plots.back().model={{v[7],v[3]}};
  out.note+="cW aus begrenzter monotoner RK4-Suche; Fallzeit und dt/2 geprueft. ";
  return true;
}
int operation(const std::string& id){
  const char* ids[]={"S_RL","S_RL_R","S_SPRING","S_SPRING_COMPARE","S_SPRING_FR","S_FALL","S_FALL_CW"};
  for(int i=0;i<7;++i)if(id==ids[i])return i;
  return -1;
}
#ifndef PHYSIK_DATA_CATALOG
Parameter p(const char* name,const char* unit){return {name,unit,unit};}
Workflow workflow(const char* id,const char* title,const char* formula,
                  std::vector<Parameter> parameters,std::vector<std::string> defaults){
  return {{title,formula,"","",id,std::move(parameters)},std::move(defaults),0,0,{},{}};
}
#endif
bool finish(const NativeResult& r){
  for(const auto& x:r.values)if(!std::isfinite(x.value))return false;
  for(const auto& p:r.plots){
    if(p.points.size()>400||p.model.size()>400)return false;
    for(const auto* series:{&p.points,&p.model})for(auto xy:*series)
      if(!std::isfinite(xy.first)||!std::isfinite(xy.second))return false;
  }
  return true;
}
}

std::vector<NativeCategory> simulationCategories(){
  return {{"S_RL_MENU","Simulation: Spule (2013)"},{"S_SPRING_MENU","Simulation: Rollwagen (2014)"},
          {"S_FALL_MENU","Simulation: Fallkegel (2016)"}};
}
#ifndef PHYSIK_DATA_CATALOG
std::vector<Workflow> simulationCatalog(const std::string& category){
  if(category=="S_RL_MENU")return {
    workflow("S_RL","Spule: I(t), RK4","I'=(U-R*I)/L",
      {p("U","V"),p("R","Ohm"),p("L","H"),p("I0","A"),p("t Ende","s"),p("dt","s")},
      {"15","7","1.5","0","1","0.001"}),
    workflow("S_RL_R","Spule: R aus I(t)","I(t Mess)=I Mess; R min..max",
      {p("U","V"),p("L","H"),p("I0","A"),p("t Mess","s"),p("I Mess","A"),p("R min","Ohm"),p("R max","Ohm"),p("t Ende","s"),p("dt","s")},
      {"15","1.5","0","0.5","1.935060068862","0.1","50","1","0.001"})};
  if(category=="S_SPRING_MENU"){
    auto direct=workflow("S_SPRING","Wagen: v(s), s(t), RK4","s'=v; m*v'=D*max(c-s,0)-FR",
      {p("m","kg"),p("D","N/m"),p("Spannung c","m"),p("FR","N"),p("s0","m"),p("v0","m/s"),p("t Ende","s"),p("dt","s")},
      {"0.05","25","0.04","0.02","0","0","5","0.001"});
    auto comparison=workflow("S_SPRING_COMPARE","Wagen: Reibung vergleichen","Gleiche Startwerte, 1..4 Reibungskraefte",
      {p("Anzahl FR",""),p("m","kg"),p("D","N/m"),p("Spannung c","m"),p("s0","m"),p("v0","m/s"),p("t Ende","s"),p("dt","s")},
      {"2","0.05","25","0.04","0","0","5","0.001"});
    comparison.minCount=1;comparison.maxCount=4;comparison.repeated={p("FR","N")};comparison.repeatedDefaults={"0.02"};
    return {std::move(direct),std::move(comparison),
      workflow("S_SPRING_FR","Wagen: FR aus Stillstand","s Stopp(FR)=s Ziel; FR min..max",
        {p("m","kg"),p("D","N/m"),p("Spannung c","m"),p("s0","m"),p("v0","m/s"),p("s Ziel","m"),p("FR min","N"),p("FR max","N"),p("t Ende","s"),p("dt","s")},
        {"0.05","25","0.04","0","0","0.59","0.005","0.5","5","0.001"})};
  }
  if(category=="S_FALL_MENU")return {
    workflow("S_FALL","Fallkegel: F(s), WR(s), s(t)","s'=v; v'=g-rho*cW*A*v*abs(v)/(2*m)",
      {p("m","kg"),p("A","m^2"),p("rho","kg/m^3"),p("cW",""),p("h","m"),p("s0","m"),p("v0","m/s"),p("g","m/s^2"),p("t Ende","s"),p("dt","s"),p("Messweg","m")},
      {"0.0018","0.0178","1.29","0.6","2","0","0","9.81","3","0.001","1"}),
    workflow("S_FALL_CW","Fallkegel: cW aus Fallzeit","t Boden(cW)=t Mess; cW min..max",
      {p("m","kg"),p("A","m^2"),p("rho","kg/m^3"),p("h","m"),p("s0","m"),p("v0","m/s"),p("g","m/s^2"),p("t Mess","s"),p("cW min",""),p("cW max",""),p("t Ende","s"),p("dt","s"),p("Messweg","m")},
      {"0.0018","0.0178","1.29","2","0","0","9.81","1.85","0.1","2","3","0.001","1"})};
  return {};
}
#endif
bool simulationCalculate(const std::string& id,const std::vector<double>& v,NativeResult& result,std::string& error){
  result={};error.clear();
  const int op=operation(id);const unsigned arity[]={6,9,8,8,10,11,13};
  if(op<0){error="Unbekanntes Simulationsmodell";return false;}
  size_t expected=arity[op];
  if(op==3){
    if(v.empty()||!std::isfinite(v[0])||v[0]!=std::floor(v[0])||v[0]<1||v[0]>4){
      error="Anzahl Reibungskraefte muss ganzzahlig zwischen 1 und 4 liegen";return false;
    }
    expected+=static_cast<unsigned>(v[0]);
  }
  if(v.size()!=expected){error="Falsche Anzahl Simulationseingaben";return false;}
  for(double x:v)if(!std::isfinite(x)){error="Simulation benoetigt endliche Eingaben";return false;}
  NativeResult out;Budget b;double worst=0;bool ok=false;
  switch(op){
    case 0:ok=rlResult(v,out,worst,b);break;
    case 1:ok=inverseRL(v,out,worst,b);break;
    case 2:ok=springResult(v,out,worst,b);break;
    case 3:
      ok=true;
      for(unsigned i=0;i<static_cast<unsigned>(v[0])&&ok;++i)
        ok=springResult({v[1],v[2],v[3],v[8+i],v[4],v[5],v[6],v[7]},out,worst,b," #"+std::to_string(i+1));
      break;
    case 4:ok=inverseSpring(v,out,worst,b);break;
    case 5:ok=fallResult(v,out,worst,b);break;
    case 6:ok=inverseFall(v,out,worst,b);break;
  }
  if(!ok){error=b.error.empty()?"Simulation nicht moeglich":b.error;return false;}
  value(out,"Abweichung dt/2 maximal",100*worst,"%");value(out,"RK4-Schritte gesamt",b.steps,"");
  if(worst>tolerance)out.note+="WARNUNG: Abweichung > 0.1% (dt/2, Modell- oder Energiekontrolle); dt verkleinern. ";
  else out.note+="dt/2-, Modell- und Energiekontrolle <= 0.1%. ";
  if(!finish(out)){error="Simulationsergebnis ausserhalb des Zahlenbereichs";return false;}
  result=std::move(out);return true;
}
}

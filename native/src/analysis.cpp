// SPDX-License-Identifier: GPL-3.0-or-later
#include "extensions.hpp"
#include <algorithm>
#include <cmath>
#include <limits>

namespace physik {
namespace {
using Points=std::vector<std::pair<double,double>>;
enum Kind { graph,velocity,force,flux,linear,power,exponential,coulomb,
  coulombPower,coil,em,decay,damping,moseley,millikanQ,millikanM,millikanR,
  planckFU,planckFE,planckLU,planckLE,seriesRL,seriesRLC,freeSpring,freeRLC,drivenSpring };
#ifdef PHYSIK_DATA_CATALOG
struct Entry {
  const char* id;Kind kind;
  constexpr Entry(const char*,const char* key,const char*,Kind k):id(key),kind(k){}
};
#else
struct Entry { const char* category;const char* id;const char* title;Kind kind; };
#endif
constexpr Entry analysisEntries[]={
  {"A_GRAPH","A_POINTS","Punkte: Sekanten und Flaechen",graph},
  {"A_GRAPH","A_VELOCITY","v(t): Beschleunigung und Weg",velocity},
  {"A_GRAPH","A_FORCE","F(t): Impulsaenderung",force},
  {"A_GRAPH","A_FLUX","Phi(t): Induktionsspannung",flux},
  {"A_FITS","A_LINEAR","Linearer Fit mit Achsenabschnitt",linear},
  {"A_FITS","A_POWER","Potenzfit (log-log)",power},
  {"A_FITS","A_EXP","Exponentialfit mit festem Untergrund",exponential},
  {"A_FITS","A_COULOMB","Coulomb: F gegen 1/r^2",coulomb},
  {"A_FITS","A_COULOMB_POWER","Coulomb: freier Exponent",coulombPower},
  {"A_FITS","A_COIL","Spule: B(I) mit Offset",coil},
  {"A_FITS","A_EM","e/m: U gegen (I*r)^2",em},
  {"A_FITS","A_DECAY","Zerfall mit festem Untergrund",decay},
  {"A_FITS","A_DAMPING","Daempfung: Amplitudenhuelle",damping},
  {"A_FITS","A_MOSELEY","Moseley: sqrt(1/lambda) gegen Z",moseley},
  {"A_MILLIKAN","A_MILLIKAN_Q","Millikan: Ladungen mit Unsicherheit",millikanQ},
  {"A_MILLIKAN","A_MILLIKAN_M","Millikan: Masse und Schwebespannung",millikanM},
  {"A_MILLIKAN","A_MILLIKAN_R","Millikan: Radius und Schwebespannung",millikanR},
  {"A_PLANCK","A_PLANCK_F_U","Planck: Frequenz / Gegenspannung",planckFU},
  {"A_PLANCK","A_PLANCK_F_E","Planck: Frequenz / Energie",planckFE},
  {"A_PLANCK","A_PLANCK_L_U","Planck: Wellenlaenge / Gegenspannung",planckLU},
  {"A_PLANCK","A_PLANCK_L_E","Planck: Wellenlaenge / Energie",planckLE},
  {"A_AC","A_RL","Wechselstrom: RL in Reihe",seriesRL},
  {"A_AC","A_RLC","Wechselstrom: RLC in Reihe",seriesRLC},
  {"A_AC","A_FREE_SPRING","Freier gedaempfter Federschwinger",freeSpring},
  {"A_AC","A_FREE_RLC","Freier RLC-Kreis mit Anfangswerten",freeRLC},
  {"A_AC","A_DRIVEN_SPRING","Erzwungene Federschwingung / Resonanz",drivenSpring}
};
#ifndef PHYSIK_DATA_CATALOG
constexpr const char* formulas[]={
  "Dimensionslos: m=dy/dx; A=sum((y0+y1)*dx/2)",
  "a=dv/dt; dx=Integral v(t) dt (vorzeichenbehaftet)",
  "dp=Integral F(t) dt; Sekante dF/dt",
  "U_ind=-N*dPhi/dt; Phi je Windung",
  "Dimensionslos: y=m*x+b, ungewichtete Regression",
  "Dimensionslos: y=A*x^p; ln(y)=ln(A)+p*ln(x)",
  "y dimensionslos, t in s: y=B+A*exp(k*t), B fest",
  "F=a/r^2+b; epsilon0=Q^2/(4*pi*a)",
  "F=A*r^p; ln(F)=ln(A)+p*ln(r), r in m, F in N",
  "B=(B/I)*I+B_offset",
  "U=m_fit*(I*r)^2+b; e/m=2*m_fit/(B/I)^2",
  "y=B+A*exp(-lambda*t); T_half=ln(2)/lambda",
  "A(t)=B+A0*exp(-delta*t); tau=1/delta",
  "sqrt(1/lambda)=m*Z+b; Abschirmung=-b/m",
  "q/e; n=round(q/e); Abw=q/e-n (festes SI-e)",
  "|q|=m*g*d/|U|; Gauss-Fehlerfortpflanzung",
  "m=4*pi*r^3*rho/3; |q|=m*g*d/|U|",
  "e*U_G=h*f-W_A; lambda_G=c/f_G (freier h-Fit)",
  "E_kin=h*f-W_A; f_G=W_A/h",
  "e*U_G=h*c/lambda-W_A; lambda_G=c*h/W_A",
  "E_kin=h*c/lambda-W_A; lambda_G=c*h/W_A",
  "Z=hypot(R,w*L); phi=atan2(w*L,R); I=U_eff/Z",
  "X=w*L-1/(w*C); Z=hypot(R,X); phi=atan2(X,R)",
  "m*x''+b*x'+D*x=0; x(0),v(0) frei",
  "L*q''+R*q'+q/C=0; q(0),I(0) frei, I=q'",
  "A=F0/hypot(D-m*w^2,b*w); w_max^2=w0^2-2*delta^2"
};
Parameter par(const char* name,const char* unit="1",const char* initial=nullptr){
  return {name,unit,initial?initial:unit};
}
Workflow workflow(const Entry& e){
  Workflow w;w.calculation={e.title,formulas[e.kind],"Ergebnisse","1",e.id,{}};
  auto& p=w.calculation.parameters;
  if(e.kind<=planckLE){
    w.minCount=2;w.maxCount=40;p={par("Punktzahl")};w.defaults={"3"};
    w.repeated={par("x"),par("y")};w.repeatedDefaults={"1","1"};
  }
  const Kind k=e.kind;
  if(k<=flux){
    if(k==flux){p.push_back(par("Windungen N"));w.defaults.push_back("100");}
    const char* xu=k==graph?"1":"s";
    p.push_back(par("Intervall von",xu));p.push_back(par("Intervall bis",xu));
    w.defaults.push_back("1");w.defaults.push_back("2");
    w.repeated={par(k==graph?"x":"t",xu),
      par(k==velocity?"v":k==force?"F":k==flux?"Phi":"y",
          k==velocity?"m/s":k==force?"N":k==flux?"Wb":"1")};
  }else if(k==exponential||k==decay||k==damping){
    p.push_back(par("Fester Untergrund",k==damping?"m":"1"));w.defaults.push_back("0");
    w.repeated={par("t","s"),par(k==damping?"Amplitude":"y",k==damping?"m":"1")};
  }else if(k==coulomb||k==coulombPower){
    w.repeated={par("r","m","cm"),par("F","N","mN")};
    if(k==coulomb){p.push_back(par("Kugelladung |Q|","C","nC"));w.defaults.push_back("37");}
    w.repeatedDefaults={"6","3.38"};
  }else if(k==coil){
    w.repeated={par("I","A"),par("B","T","mT")};
  }else if(k==em){
    p.push_back(par("Bekanntes B/I","T/A"));w.defaults.push_back("0.0007");
    w.repeated={par("I","A"),par("r","m","cm"),par("U","V")};
    w.repeatedDefaults={"1","5","200"};
  }else if(k==moseley){
    w.repeated={par("Ordnungszahl Z"),par("lambda","m","nm")};
    w.repeatedDefaults={"29","0.154"};
  }else if(k>=millikanQ&&k<=millikanR){
    if(k==millikanQ){
      w.repeated={par("q","C"),par("sigma_q","C")};w.repeatedDefaults={"1.6e-19","0"};
    }else{
      p.push_back(par("Plattenabstand d","m","mm"));
      p.push_back(par("g","m/s^2"));p.push_back(par("sigma_d","m","mm"));
      p.push_back(par("sigma_g","m/s^2"));w.defaults.insert(w.defaults.end(),{"1","9.81","0","0"});
      if(k==millikanR){p.push_back(par("Dichte","kg/m^3"));p.push_back(par("sigma_Dichte","kg/m^3"));
        w.defaults.insert(w.defaults.end(),{"2300","0"});}
      w.repeated={par(k==millikanR?"Radius":"Masse",k==millikanR?"m":"kg",k==millikanR?"um":"kg"),
        par("Schwebespannung U","V"),
        par(k==millikanR?"sigma_r":"sigma_m",k==millikanR?"m":"kg",k==millikanR?"um":"kg"),par("sigma_U","V")};
      w.repeatedDefaults={k==millikanR?"1":"9.634217471e-15","115","0","0"};
    }
  }else if(k>=planckFU&&k<=planckLE){
    const bool lambda=k==planckLU||k==planckLE,energy=k==planckFE||k==planckLE;
    w.maxCount=10;w.defaults={"4"};
    w.repeated={par(lambda?"lambda":"f",lambda?"m":"Hz",lambda?"nm":"THz"),
      par(energy?"E_kin":"U_G",energy?"J":"V",energy?"eV":"V")};
    w.repeatedDefaults={lambda?"365":"821.3492","1.15"};
  }else if(k==seriesRL||k==seriesRLC){
    p={par("R","Ohm"),par("L","H"),par("f","Hz"),par("U_eff","V"),par("t","s")};
    w.defaults={"22","0.5","16.6666666667","250","0"};
    if(k==seriesRLC){p.insert(p.begin()+2,par("C","F","uF"));w.defaults={"22","1.2","15","50","220","0"};}
  }else if(k==freeSpring||k==freeRLC){
    const bool electric=k==freeRLC;
    p={par(electric?"L":"m",electric?"H":"kg"),par(electric?"C":"D",electric?"F":"N/m"),
      par(electric?"R":"Reibkoeffizient b",electric?"Ohm":"kg/s"),
      par(electric?"q(0)":"x(0)",electric?"C":"m"),par(electric?"I(0)":"v(0)",electric?"A":"m/s"),
      par("Auswertezeit t","s"),par("Diagramm bis","s")};
    w.defaults=electric?std::vector<std::string>{"1","0.0001","10","0.001","0","0","1"}:
      std::vector<std::string>{"1","4","0.4","0.1","0","1","10"};
  }else if(k==drivenSpring){
    p={par("m","kg"),par("D","N/m"),par("Reibkoeffizient b","kg/s"),par("Kraftamplitude","N"),
      par("Erregerfrequenz","Hz"),par("Diagramm bis","Hz")};w.defaults={"1","4","0.4","1","0.3","1"};
  }
  return w;
}
#endif
bool bad(std::string& error,const char* text){error=text;return false;}
void add(NativeResult& r,const std::string& name,double value,const std::string& unit="1"){
  r.values.push_back({name,value,unit});
}
Plot plot(const char* title,const char* x,const char* xu,const char* y,const char* yu,bool joined=true){
  return {title,x,xu,y,yu,{},{},joined};
}
bool finish(NativeResult& out,NativeResult& result,std::string& error){
  for(const auto& v:out.values)if(!std::isfinite(v.value))return bad(error,"Ergebnis ausserhalb des Zahlenbereichs");
  for(const auto& p:out.plots){
    if(p.points.size()>400||p.model.size()>400)return bad(error,"Diagramm hat zu viele Punkte");
    for(const auto* list:{&p.points,&p.model})for(auto [x,y]:*list)
      if(!std::isfinite(x)||!std::isfinite(y))return bad(error,"Diagramm ausserhalb des Zahlenbereichs");
  }
  result=std::move(out);error.clear();return true;
}
// Scale before centering: SI data include squared ampere-metres and Planck energies.
struct Line { double a=0,b=0,sa=0,sb=0,cov=0,rms=0,r2=0;bool constant=false; };
bool fitLine(const Points& p,Line& s,std::string& error){
  double xs=0,ys=0;
  for(auto [x,y]:p){if(!std::isfinite(x)||!std::isfinite(y))return bad(error,"Fitwerte ausserhalb des Zahlenbereichs");
    xs=std::max(xs,std::abs(x));ys=std::max(ys,std::abs(y));}
  if(xs==0||std::all_of(p.begin(),p.end(),[&](const auto& v){return v.first==p.front().first;}))
    return bad(error,"Verschiedene x-Werte erforderlich");
  if(ys==0)ys=1;
  const double n=static_cast<double>(p.size());double xm=0,ym=0;
  for(auto [x,y]:p){xm+=(x/xs)/n;ym+=(y/ys)/n;}
  double xx=0,xy=0,yy=0;
  for(auto [x,y]:p){const double dx=x/xs-xm,dy=y/ys-ym;xx+=dx*dx;xy+=dx*dy;yy+=dy*dy;}
  if(!(xx>0)||!std::isfinite(xx))return bad(error,"Verschiedene x-Werte erforderlich");
  if(std::all_of(p.begin(),p.end(),[&](const auto& v){return v.second==p.front().second;})){
    s.constant=true;s.b=p.front().second;return true;
  }
  const double a=xy/xx,b=ym-a*xm;double ss=0;
  for(auto [x,y]:p){const double d=y/ys-(a*(x/xs)+b);ss+=d*d;}
  s.a=(a*ys)/xs;s.b=b*ys;s.rms=std::sqrt(ss/n)*ys;s.constant=yy==0;
  if(!s.constant)s.r2=1-ss/yy;
  if(p.size()>2){const double variance=ss/(n-2);
    s.sa=std::sqrt(variance/xx)*ys/xs;s.sb=std::sqrt(variance*(1/n+xm*xm/xx))*ys;
    s.cov=-xm*(variance/xx)*ys*(ys/xs);}
  return std::isfinite(s.a)&&std::isfinite(s.b)&&std::isfinite(s.rms)&&std::isfinite(s.r2)
    ?true:bad(error,"Fit ausserhalb des Zahlenbereichs");
}
void quality(NativeResult& r,const Line& s,const char* rms,const char* r2,const char* unit){
  add(r,rms,s.rms,unit);
  if(s.constant)r.note+=std::string(r2)+": bei konstantem y nicht definiert. ";
  else add(r,r2,s.r2);
}
void statistics(NativeResult& r,size_t count){
  r.note+=count>2?"1-sigma-Parameterfehler aus ungewichteter Restkovarianz (n-2); keine experimentellen/systematischen Unsicherheiten. ":
    "2 Punkte: keine Restfreiheitsgrade, keine Kovarianz und keine statistischen Parameterfehler. ";
}
bool originalQuality(NativeResult& r,const Points& data,const Points& model,const char* unit){
  double scale=0;for(auto [x,y]:data){(void)x;scale=std::max(scale,std::abs(y));}
  if(scale==0)scale=1;
  double mean=0;const double n=static_cast<double>(data.size());
  for(auto [x,y]:data){(void)x;mean+=(y/scale)/n;}
  double ss=0,total=0;
  for(size_t i=0;i<data.size();++i){const double y=data[i].second/scale,d=y-model[i].second/scale;
    ss+=d*d;total+=(y-mean)*(y-mean);}
  Line s;s.rms=std::sqrt(ss/n)*scale;
  s.constant=std::all_of(data.begin(),data.end(),[&](const auto& v){return v.second==data.front().second;});
  if(!s.constant)s.r2=1-ss/total;
  quality(r,s,"RMS_original","R2_original",unit);return std::isfinite(s.rms)&&std::isfinite(s.r2);
}
bool graphAnalysis(Kind k,const std::vector<double>& v,size_t base,NativeResult& r,std::string& error){
  const size_t count=static_cast<size_t>(v[0]);const size_t offset=k==flux?2:1;
  const double lo=v[offset],hi=v[offset+1],factor=k==flux?-v[1]:1;
  if(k==flux&&(v[1]<1||v[1]!=std::floor(v[1])))return bad(error,"Windungszahl muss positiv und ganzzahlig sein");
  Points p;for(size_t i=0;i<count;++i){const double x=v[base+2*i],y=v[base+2*i+1];
    if((k!=graph&&x<0)||(i&&x<=p.back().first))return bad(error,"x streng steigend; Zeiten nicht negativ eingeben");
    p.emplace_back(x,y);}
  if(!(lo<hi)||lo<p.front().first||hi>p.back().first)return bad(error,"Intervall muss innerhalb der Messreihe liegen (von < bis)");
  const char* xu=k==graph?"1":"s",*yu=k==velocity?"m/s":k==force?"N":k==flux?"Wb":"1";
  const char* du=k==velocity?"m/s^2":k==force?"N/s":k==flux?"V":"1";
  const char* iu=k==velocity?"m":k==force?"N*s":k==flux?"Wb*s":"1";
  Plot raw=plot("Messpunkte (linear verbunden)",k==graph?"x":"t",xu,k==velocity?"v":k==force?"F":k==flux?"Phi":"y",yu);
  Plot derivative=plot("Sekanten: stueckweise konstant",k==graph?"x":"t",xu,k==flux?"U_ind":k==velocity?"a":k==force?"dF/dt":"dy/dx",du);
  Plot integral=plot("Kumulatives Integral ab erstem Punkt",k==graph?"x":"t",xu,k==velocity?"dx":k==force?"dp":"Integral",iu);
  raw.points=p;integral.points.emplace_back(p.front().first,0);
  double area=0,selected=0,selectedChange=0;
  for(size_t i=1;i<p.size();++i){const auto [x0,y0]=p[i-1];const auto [x1,y1]=p[i];
    const double dx=x1-x0,slope=(y1-y0)/dx,trapezoid=(y0/2+y1/2)*dx;
    if(!(dx>0)||!std::isfinite(slope))return bad(error,"Sekante ausserhalb des Zahlenbereichs");
    add(r,"m"+std::to_string(i),factor*slope,du);add(r,"A"+std::to_string(i),trapezoid,iu);
    derivative.points.emplace_back(x0,factor*slope);derivative.points.emplace_back(x1,factor*slope);
    for(unsigned j=1;j<=4;++j){const double h=dx*(j/4.0);
      integral.points.emplace_back(j==4?x1:x0+h,area+h*(y0+0.5*slope*h));}
    area+=trapezoid;
    const double a=std::max(lo,x0),b=std::min(hi,x1);
    if(a<b){const double ya=y0+slope*(a-x0),yb=y0+slope*(b-x0);
      selected+=(ya/2+yb/2)*(b-a);selectedChange+=yb-ya;}
  }
  add(r,k==velocity?"dx":k==force?"dp":"Flaeche",selected,iu);
  add(r,"Flaeche_gesamt",area,iu);
  add(r,k==velocity?"a_mittel":k==flux?"U_mittel":"Sekante",factor*selectedChange/(hi-lo),du);
  r.plots={std::move(raw),std::move(derivative),std::move(integral)};
  r.note="Stueckweise lineare Interpolation; Flaechen vorzeichenbehaftet, keine Betragsflaechen. Ableitung als Treppenkurve, Integral mit Zwischenpunkten. ";
  if(k==graph)r.note+="x und y dimensionslose Zahlen (Einheit 1). Physikalische Achsen: Presets v(t), F(t), Phi(t). ";
  if(k==flux)r.note+="U_ind=-N*dPhi/dt, positives Phi nach gewaehlter Flaechennormale. ";
  return true;
}
bool regression(Kind k,const std::vector<double>& v,size_t base,NativeResult& r,std::string& error){
  const size_t n=static_cast<size_t>(v[0]);Points raw,transformed;
  const bool pow=k==power||k==coulombPower,ex=k==exponential||k==decay||k==damping,logs=pow||ex;
  const double background=ex?v[1]:0;
  if((k==decay||k==damping)&&background<0)return bad(error,"Zerfall/Amplitude: Untergrund muss >= 0 sein");
  if((k==em||k==coulomb)&&v[1]<=0)return bad(error,"B/I bzw. Kugelladung muss positiv sein");
  for(size_t i=0;i<n;++i){double x=v[base+(k==em?3:2)*i],y=v[base+(k==em?3:2)*i+1];
    if(k==em){const double radius=y;y=v[base+3*i+2];
      if(x<=0||radius<=0||y<0)return bad(error,"e/m: I, r > 0 und U >= 0 erforderlich");
      x=(x*radius)*(x*radius);}
    if((k==coulomb||k==coulombPower)&&(x<=0||y<0))return bad(error,"Coulomb: Abstand > 0 und Kraftbetrag >= 0");
    if((k==decay||k==damping)&&x<0)return bad(error,"Zerfalls- und Daempfungszeiten muessen >= 0 sein");
    if(k==moseley&&(x<1||x>118||x!=std::floor(x)||y<=0))return bad(error,"Moseley: Z ganzzahlig 1..118, lambda > 0");
    raw.emplace_back(x,y);
    if(pow){if(x<=0||y<=0)return bad(error,"Log-log-Fit erfordert x > 0 und y > 0");x=std::log(x);y=std::log(y);}
    if(ex){if(!(y>background))return bad(error,"Messwert muss ueber dem festen Untergrund liegen");y=std::log(y-background);}
    if(k==coulomb)x=(1/x)/x;
    if(k==moseley)y=1/std::sqrt(y);
    transformed.emplace_back(x,y);
  }
  Line s;if(!fitLine(transformed,s,error))return false;
  if((k==coulomb||k==em||k==moseley)&&s.a<=0)return bad(error,"Physikalischer Fit erfordert positive Steigung");
  if((k==decay||k==damping)&&s.a>=0)return bad(error,"Zerfall/Daempfung erfordert fallende Amplitudenhuelle");
  const char* xu=k==coil?"A":k==em?"A^2*m^2":k==coulomb||k==coulombPower?"m":ex?"s":"1";
  const char* yu=k==coil?"T":k==em?"V":k==coulomb||k==coulombPower?"N":k==moseley||k==damping?"m":"1";
  Plot p=plot("Messwerte und Fit (Originalraum)",k==moseley?"Z":k==coil?"I":k==em?"(I*r)^2":k==coulomb||k==coulombPower?"r":ex?"t":"x",xu,
    k==moseley?"lambda":k==coil?"B":k==em?"U":k==coulomb||k==coulombPower?"F":k==damping?"Amplitude":"y",yu,false);p.points=raw;
  auto model=[&](double x){
    if(pow)return std::exp(s.b+s.a*std::log(x));
    if(ex)return background+std::exp(s.b+s.a*x);
    if(k==coulomb)return s.a*((1/x)/x)+s.b;
    if(k==moseley){const double t=s.a*x+s.b;return t>0?(1/t)/t:std::numeric_limits<double>::quiet_NaN();}
    return s.a*x+s.b;
  };
  Points atData;
  for(auto [x,y]:raw){(void)y;atData.emplace_back(x,model(x));}
  double lo=raw[0].first,hi=lo;for(auto [x,y]:raw){(void)y;lo=std::min(lo,x);hi=std::max(hi,x);}
  for(unsigned j=0;j<=80;++j){const double x=lo+(hi-lo)*(j/80.0);p.model.emplace_back(x,model(x));}
  r.plots.push_back(std::move(p));statistics(r,n);
  if(k==linear||k==power)r.note+="Generischer Fit: x und y dimensionslos (Einheit 1); physikalische Einheiten ueber Coulomb-, Spulen-, e/m-, Zerfalls- und Daempfungspresets. ";
  if(k==exponential)r.note+="Generischer Exponentialfit: t in s, y und fester Untergrund dimensionslos (Einheit 1). ";
  if(logs){
    const double A=std::exp(s.b);
    if(!(A>0))return bad(error,"Fitamplitude ausserhalb des Zahlenbereichs");
    const std::string aUnit=k==coulombPower?"N*m^("+formatNumber(-s.a)+")":yu;
    add(r,"A",A,aUnit);add(r,pow?"p":"k",s.a,pow?"1":"1/s");
    if(ex)add(r,"Untergrund",background,yu);
    if(n>2){add(r,"sigma_A",A*s.sb,aUnit);add(r,pow?"sigma_p":"sigma_k",s.sa,pow?"1":"1/s");}
    quality(r,s,"RMS_log","R2_log","1");
    if(!originalQuality(r,raw,atData,yu))return bad(error,"Fitguete ausserhalb des Zahlenbereichs");
    r.note+="Log-Fit minimiert Residuen im transformierten Raum; RMS/R2_log beziehen sich auf ln(y-Untergrund) bzw. ln(y), RMS/R2_original auf Originalwerte. A bezieht sich auf x in SI. ";
    if(k==decay||k==damping){const double rate=-s.a;
      add(r,k==decay?"lambda":"delta",rate,"1/s");add(r,"tau",1/rate,"s");
      if(k==decay)add(r,"T_half",std::log(2.0)/rate,"s");
      if(n>2){add(r,"sigma_tau",s.sa/(rate*rate),"s");if(k==decay)add(r,"sigma_T_half",std::log(2.0)*s.sa/(rate*rate),"s");}
      if(k==damping)r.note+="Nur positive Amplitudenmaxima/Huelle, keine vorzeichenwechselnden Schwingungswerte fitten. ";
    }
  }else{
    const char* au=k==coil?"T/A":k==em?"V/(A^2*m^2)":k==coulomb?"N*m^2":k==moseley?"1/sqrt(m)":"1";
    const char* bu=k==moseley?"1/sqrt(m)":yu;
    add(r,"m",s.a,au);add(r,"b",s.b,bu);
    if(n>2){add(r,"sigma_m",s.sa,au);add(r,"sigma_b",s.sb,bu);}
    quality(r,s,"RMS","R2",bu);
    if(k==coulomb||k==moseley){
      Plot transformedPlot=plot("Linearisierter Fit",k==coulomb?"1/r^2":"Z",k==coulomb?"1/m^2":"1",
        k==coulomb?"F":"sqrt(1/lambda)",bu,false);
      transformedPlot.points=transformed;
      for(auto [x,y]:transformed){(void)y;transformedPlot.model.emplace_back(x,s.a*x+s.b);}
      std::sort(transformedPlot.model.begin(),transformedPlot.model.end());r.plots.push_back(std::move(transformedPlot));
      r.note+="RMS und R2 im linearisierten Raum. ";
    }
    if(k==coulomb){const double eps=(v[1]/s.a)*v[1]/(4*pi);
      if(!(eps>0))return bad(error,"Permittivitaet ausserhalb des Zahlenbereichs");
      add(r,"epsilon_0_exp",eps,"F/m");add(r,"Abweichung_epsilon",100*(eps/8.8541878128e-12-1),"%");
      if(n>2)add(r,"sigma_epsilon",eps*s.sa/s.a,"F/m");
      r.note+="F=a/r^2+b mit freiem Offset; Theorie b=0. Gleiche Kugelladungen; Influenz bei kleinen Abstaenden pruefen. ";
    }
    if(k==em){const double ratio=2*(s.a/v[1])/v[1];
      if(!(ratio>0))return bad(error,"e/m ausserhalb des Zahlenbereichs");
      add(r,"e/m",ratio,"C/kg");
      if(n>2)add(r,"sigma_e/m",2*(s.sa/v[1])/v[1],"C/kg");
      r.note+="U=m_fit*(I*r)^2+b, e/m=2*m_fit/(B/I)^2; bekanntes Spulenfeld, nichtrelativistische Kreisbahn. ";}
    if(k==coil)r.note+="B=(B/I)*I+B_offset; Offset nicht auf null gezwungen. ";
    if(k==moseley){add(r,"Abschirmung",-s.b/s.a);add(r,"R_eff",s.a*s.a,"1/m");
      r.note+="sqrt(1/lambda)=m*Z+b, Abschirmung=-b/m. R_eff=m^2 enthaelt den Serienfaktor (K-alpha: 3/4); nicht direkt R_H. ";}
  }
  return true;
}
bool millikan(Kind k,const std::vector<double>& v,size_t base,NativeResult& r,std::string& error){
  const size_t n=static_cast<size_t>(v[0]);
  if(k!=millikanQ&&(v[1]<=0||v[2]<=0||v[3]<0||v[4]<0))return bad(error,"Millikan: d,g > 0; Unsicherheiten >= 0");
  if(k==millikanR&&(v[5]<=0||v[6]<0))return bad(error,"Dichte > 0, Dichteunsicherheit >= 0 erforderlich");
  Plot columns=plot("Ladungen: Saeulendiagramm","Messung","1","q/e","1");Points charges;bool anySigma=false;
  for(size_t i=0;i<n;++i){const size_t j=base+(k==millikanQ?2:4)*i;double q=v[j],sigma=v[j+1];
    if(k!=millikanQ){const double a=v[j],U=v[j+1],sa=v[j+2],sU=v[j+3];
      if(a<=0||U==0||sa<0||sU<0)return bad(error,"Radius/Masse > 0, U != 0, Unsicherheiten >= 0 erforderlich");
      const double mass=k==millikanR?((4*pi/3)*a*a)*a*v[5]:a;
      if(!(mass>0))return bad(error,"Masse ausserhalb des Zahlenbereichs");
      q=((mass*v[2])*v[1])/std::abs(U);
      if(!(q>0))return bad(error,"Ladung ausserhalb des Zahlenbereichs");
      double relative=std::hypot((k==millikanR?3:1)*sa/a,sU/std::abs(U));
      relative=std::hypot(relative,v[3]/v[1]);relative=std::hypot(relative,v[4]/v[2]);
      if(k==millikanR)relative=std::hypot(relative,v[6]/v[5]);
      sigma=q*relative;
    }
    if(sigma<0||!std::isfinite(q)||!std::isfinite(sigma))return bad(error,"Ladung/Unsicherheit ausserhalb des Zahlenbereichs");
    const double ratio=q/charge,nearest=std::round(ratio),deviation=ratio-nearest;
    if(!std::isfinite(ratio))return bad(error,"q/e ausserhalb des Zahlenbereichs");
    const std::string suffix="_"+std::to_string(i+1);add(r,"q"+suffix,q,"C");add(r,"q/e"+suffix,ratio);
    add(r,"n"+suffix,nearest);add(r,"Abw"+suffix,deviation);add(r,"sigma_q"+suffix,sigma,"C");
    if(sigma>0){anySigma=true;add(r,"sigma_q/e"+suffix,sigma/charge);add(r,"Abw/sigma"+suffix,(q-nearest*charge)/sigma);}
    const double x=static_cast<double>(i+1);
    columns.points.insert(columns.points.end(),{{x-.3,0},{x-.3,ratio},{x+.3,ratio},{x+.3,0},{x-.3,0}});
    charges.emplace_back(ratio,0);
  }
  double lo=charges.front().first,hi=lo;for(auto [q,z]:charges){(void)z;lo=std::min(lo,q);hi=std::max(hi,q);}
  if(hi==lo){lo-=.5;hi+=.5;}
  else{const double padding=(hi-lo)*.01;lo-=padding;hi+=padding;}
  constexpr unsigned bins=12;double counts[bins]{};const double width=(hi-lo)/bins;
  if(!(width>0)||!std::isfinite(width))return bad(error,"Histogrammbereich nicht darstellbar");
  for(auto [q,z]:charges){(void)z;const double index=(q-lo)/width;
    if(!std::isfinite(index))return bad(error,"Histogrammbereich nicht darstellbar");
    const unsigned bin=static_cast<unsigned>(std::max(0.0,std::min(index,static_cast<double>(bins-1))));counts[bin]+=1;}
  Plot hist=plot("Ladungshistogramm (12 gleich breite Klassen)","q/e","1","Anzahl","1");
  for(unsigned i=0;i<bins;++i){const double a=lo+i*width,b=a+width;
    hist.points.insert(hist.points.end(),{{a,0},{a,counts[i]},{b,counts[i]},{b,0},{a,0}});}
  r.plots={std::move(columns),std::move(hist)};add(r,"Messungen",static_cast<double>(n));
  r.note="Vergleich mit festem SI-e, kein freier ggT-Fit. Tabelle: q/e, naechste ganze Zahl n, Abw=q/e-n. Histogramm ohne Rundung der Messwerte. ";
  r.note+=anySigma?"Gauss-Fehlerfortpflanzung fuer unabhaengige Eingaben; gemeinsame systematische Fehler werden nicht durch Mitteln reduziert. Abw/sigma ist nur ein Vergleich, kein Quantisierungsbeweis. ":
    "Keine Messunsicherheiten angegeben: Quantisierung nicht nachgewiesen; Naehe zu ganzen Zahlen allein reicht nicht. ";
  if(k!=millikanQ)r.note+="Schwebemethode q=m*g*d/|U|: Ladungsbetrag, ohne Auftrieb; Modell und Dichte pruefen. ";
  return true;
}
bool ac(Kind k,const std::vector<double>& v,NativeResult& r,std::string& error){
  const bool rlc=k==seriesRLC;const double R=v[0],L=v[1],C=rlc?v[2]:1;
  const size_t o=rlc?3:2;const double f=v[o],U=v[o+1],t=v[o+2];
  if(R<=0||L<0||(rlc&&(L<=0||C<=0))||f<=0||U<0||t<0)
    return bad(error,"AC: R,f > 0; L,U,t >= 0; RLC zusaetzlich L,C > 0");
  const double omega=2*pi*f,XL=omega*L,XC=rlc?1/(omega*C):0,X=XL-XC,Z=std::hypot(R,X);
  const double I=U/Z,peak=std::sqrt(2.0)*I,phi=std::atan2(X,R),phase=omega*t;
  add(r,"X_L",XL,"Ohm");add(r,"X_C",XC,"Ohm");add(r,"Z",Z,"Ohm");add(r,"I_eff",I,"A");add(r,"I_hat",peak,"A");
  add(r,"U_hat",std::sqrt(2.0)*U,"V");
  add(r,"U_R",I*R,"V");add(r,"U_L",I*XL,"V");add(r,"U_C",I*XC,"V");
  add(r,"P",I*I*R,"W");add(r,"S",U*I,"VA");add(r,"Q",I*I*X,"var");
  add(r,"Q_L",I*I*XL,"var");add(r,"Q_C",-I*I*XC,"var");add(r,"phi",phi,"rad");add(r,"cos_phi",R/Z);
  add(r,"i(t)",peak*std::cos(phase),"A");add(r,"u(t)",std::sqrt(2.0)*U*std::cos(phase+phi),"V");
  add(r,"u_R(t)",peak*R*std::cos(phase),"V");add(r,"u_L(t)",peak*XL*std::cos(phase+pi/2),"V");
  add(r,"u_C(t)",peak*XC*std::cos(phase-pi/2),"V");
  Plot phasor=plot("Spannungszeiger (Effektivwerte)","Real","V","Imaginaer","V");
  phasor.points={{0,0},{I*R,0},{I*R,I*X},{0,0}};
  phasor.model={{I*R,0},{I*R,I*XL},{I*R,I*X}};r.plots.push_back(std::move(phasor));
  Plot current=plot("Strom: zwei Perioden","t","s","i","A");
  for(unsigned j=0;j<=80;++j){const double time=(2/f)*(j/80.0);current.points.emplace_back(time,peak*std::cos(2*pi*f*time));}
  r.plots.push_back(std::move(current));
  if(rlc){const double f0=(1/std::sqrt(L))/std::sqrt(C)/(2*pi);add(r,"f_0",f0,"Hz");add(r,"I_res",U/R,"A");
    Plot resonance=plot("Elektrische Stromresonanz","f","Hz","I_eff","A");
    for(unsigned j=0;j<=80;++j){const double ff=f0*(j/40.0),w=2*pi*ff;
      resonance.points.emplace_back(ff,j==0?0:U/std::hypot(R,w*L-1/(w*C)));}
    r.plots.push_back(std::move(resonance));
  }
  r.note="Ideale Serienbauteile, sinusfoermig eingeschwungen. i=I_hat*cos(w*t), u=U_hat*cos(w*t+phi), phi=atan2(XL-XC,R). Q gesamt vorzeichenbehaftet: induktiv +, kapazitiv -. ";
  r.note+=rlc?"Stromresonanz f0=1/(2*pi*sqrt(L*C)), I_res=U_eff/R; nicht das Maximum der Kondensatorspannung oder einer Federamplitude.":
    "RL ohne Kondensator: keine endliche Resonanzfrequenz.";
  return true;
}
struct Motion { double x,v; };
Motion freeMotion(double omega,double delta,double x0,double v0,double t){
  if(t==0)return {x0,v0};
  if(std::abs(delta-omega)<=omega*1e-10){const double b=v0+delta*x0,e=std::exp(-delta*t);
    return {e*(x0+b*t),e*(v0-delta*b*t)};}
  if(delta<omega){const double w=std::sqrt(omega-delta)*std::sqrt(omega+delta),e=std::exp(-delta*t),c=std::cos(w*t),s=std::sin(w*t)/w;
    return {e*(x0*c+(v0+delta*x0)*s),e*(v0*c-(delta*v0+omega*omega*x0)*s)};}
  const double d=std::sqrt(delta-omega)*std::sqrt(delta+omega),fast=-(delta+d),slow=-omega*(omega/(delta+d));
  const double a=(v0-fast*x0)/(slow-fast),b=x0-a,e1=std::exp(slow*t),e2=std::exp(fast*t);
  return {a*e1+b*e2,slow*a*e1+fast*b*e2};
}
bool oscillation(Kind k,const std::vector<double>& v,NativeResult& r,std::string& error){
  const bool electric=k==freeRLC;const double a=v[0],b=v[1],friction=v[2],x0=v[3],v0=v[4],t=v[5],end=v[6];
  if(a<=0||b<=0||friction<0||t<0||end<=0)return bad(error,"L,C bzw. m,D und Diagrammdauer > 0; Daempfung und Zeit >= 0");
  const double omega=electric?(1/std::sqrt(a))/std::sqrt(b):std::sqrt(b)/std::sqrt(a),delta=(friction/a)/2;
  if(!std::isfinite(omega)||omega<=0||!std::isfinite(delta))return bad(error,"Schwingungsparameter ausserhalb des Zahlenbereichs");
  const Motion z=freeMotion(omega,delta,x0,v0,t);const double acceleration=-2*delta*z.v-omega*omega*z.x;
  add(r,"omega_0",omega,"rad/s");add(r,"f_0",omega/(2*pi),"Hz");add(r,"delta",delta,"1/s");
  if(delta<omega)add(r,"omega_d",std::sqrt(omega-delta)*std::sqrt(omega+delta),"rad/s");
  add(r,electric?"q(t)":"x(t)",z.x,electric?"C":"m");add(r,electric?"I(t)":"v(t)",z.v,electric?"A":"m/s");
  add(r,electric?"U_C(t)":"a(t)",electric?z.x/b:acceleration,electric?"V":"m/s^2");
  if(electric)add(r,"U_L(t)",a*acceleration,"V");
  add(r,"E(t)",0.5*a*z.v*z.v+0.5*(electric?1/b:b)*z.x*z.x,"J");
  add(r,"E(0)",0.5*a*v0*v0+0.5*(electric?1/b:b)*x0*x0,"J");
  Plot position=plot(electric?"Freier Kreis: Ladung":"Freier Schwinger: Auslenkung","t","s",electric?"q":"x",electric?"C":"m");
  Plot speed=plot(electric?"Freier Kreis: Strom":"Freier Schwinger: Geschwindigkeit","t","s",electric?"I":"v",electric?"A":"m/s");
  const double needed=end*(omega/(2*pi))*20;
  const unsigned intervals=(!std::isfinite(needed)||needed>=399)?399:static_cast<unsigned>(std::max(80.0,std::ceil(needed)));
  for(unsigned i=0;i<=intervals;++i){const double tt=end*(static_cast<double>(i)/intervals);const Motion zz=freeMotion(omega,delta,x0,v0,tt);
    position.points.emplace_back(tt,zz.x);speed.points.emplace_back(tt,zz.v);}
  r.plots={std::move(position),std::move(speed)};
  r.note=std::abs(delta-omega)<=omega*1e-10?"Aperiodischer Grenzfall. ":delta<omega?"Unterkritisch gedaempft (Schwingfall). ":"Ueberkritisch gedaempft (Kriechfall). ";
  r.note+="Analytische freie Loesung mit beiden Anfangswerten, keine Anregung. ";
  if(needed>399)r.note+="Zeitdiagramm auf 400 Punkte begrenzt; fuer schnelle Schwingungen Zeitfenster verkleinern. ";
  if(electric)r.note+="L*q''+R*q'+q/C=0; I=dq/dt, U_C=q/C nach dieser Vorzeichenkonvention. ";
  else r.note+="m*x''+b*x'+D*x=0; lineare viskose Daempfung. ";
  return true;
}
bool driven(const std::vector<double>& v,NativeResult& r,std::string& error){
  const double m=v[0],D=v[1],b=v[2],F=v[3],f=v[4],end=v[5];
  if(m<=0||D<=0||b<=0||F<0||f<0||end<=0)return bad(error,"m,D,b,Diagrammfrequenz > 0; F und f >= 0 (ungedaempfte Resonanz nicht endlich)");
  const double w0=std::sqrt(D)/std::sqrt(m),delta=(b/m)/2,w=2*pi*f;
  const double wp2=w0*w0-2*delta*delta,fp=wp2>0?std::sqrt(wp2)/(2*pi):0;
  if(!std::isfinite(wp2)||!std::isfinite(w)||w0<=0)return bad(error,"Resonanzparameter ausserhalb des Zahlenbereichs");
  auto amplitude=[&](double ff){const double ww=2*pi*ff;return F/std::hypot(D-m*ww*ww,b*ww);};
  add(r,"Amplitude",amplitude(f),"m");add(r,"phi",std::atan2(b*w,D-m*w*w),"rad");add(r,"f_0",w0/(2*pi),"Hz");
  add(r,"f_Amplitude_max",fp,"Hz");add(r,"Amplitude_max",amplitude(fp),"m");
  Plot p=plot("Erzwungene Schwingung: Amplitudenresonanz","f","Hz","Amplitude","m");
  for(unsigned i=0;i<=80;++i){const double ff=end*(i/80.0);p.points.emplace_back(ff,amplitude(ff));}
  if(fp>0&&fp<end){p.points.emplace_back(fp,amplitude(fp));std::sort(p.points.begin(),p.points.end());}
  r.plots.push_back(std::move(p));
  r.note="Eingeschwungene Loesung fuer F=F0*cos(w*t), x=A*cos(w*t-phi). Amplitudenmaximum bei sqrt(w0^2-2*delta^2); bei starker Daempfung bei f=0. Anders als elektrische Stromresonanz f0; freie Schwingung hat sqrt(w0^2-delta^2). ";
  return true;
}
} // namespace

std::vector<NativeCategory> analysisCategories(){
  return {{"A_GRAPH","Diagrammpunkte"},{"A_FITS","Messreihen und Fits"},{"A_MILLIKAN","Millikan-Messreihen"},
    {"A_PLANCK","Planck-Auswertung"},{"A_AC","Schwingungen / Wechselstrom"}};
}
#ifndef PHYSIK_DATA_CATALOG
std::vector<Workflow> analysisCatalog(const std::string& category){
  std::vector<Workflow> out;for(const auto& e:analysisEntries)if(category==e.category)out.push_back(workflow(e));return out;
}
#endif
bool planckAnalysis(const Points& points,bool wavelength,NativeResult& result,std::string& error){
  result={};error.clear();Fit fit;if(!fitPlanck(points,wavelength,fit,error))return false;
  NativeResult r;add(r,"h",fit.h,"J*s");add(r,"W_A",fit.work,"J");add(r,"W_A_eV",fit.work/charge,"eV");add(r,"f_G",fit.threshold,"Hz");
  if(fit.threshold>0)add(r,"lambda_G",light/fit.threshold,"m");
  else r.note="f_G=0: lambda_G nicht endlich/undefiniert, daher nicht ausgegeben. ";
  Points scaled;for(auto [x,y]:points)scaled.emplace_back((wavelength?light/x:x)/1e12,wavelength?y/charge:y);
  Line s;if(!fitLine(scaled,s,error))return false;
  quality(r,s,"RMS","R2",wavelength?"eV":"V");statistics(r,points.size());
  if(points.size()>2){add(r,"sigma_h",s.sa*charge/1e12,"J*s");add(r,"sigma_W_A",s.sb*charge,"J");
    const double f=-s.b/s.a;
    const double variance=(s.sb*s.sb+f*f*s.sa*s.sa+2*f*s.cov)/(s.a*s.a);
    if(!std::isfinite(variance))return bad(error,"Kovarianz ausserhalb des Zahlenbereichs");
    const double sf=std::sqrt(std::max(0.0,variance))*1e12;add(r,"sigma_f_G",sf,"Hz");
    if(fit.threshold>0)add(r,"sigma_lambda_G",(light/fit.threshold)*(sf/fit.threshold),"m");
  }
  Plot p=plot("Photoeffekt: Messwerte und freie Gerade","f","THz",wavelength?"E_kin":"U_G",wavelength?"eV":"V",false);
  p.points=scaled;double lo=scaled[0].first,hi=lo;
  for(auto [x,y]:scaled){(void)y;lo=std::min(lo,x);hi=std::max(hi,x);}
  lo=std::min(lo,fit.threshold/1e12);
  p.model={{lo,(fit.h*(lo*1e12)-fit.work)/charge},{hi,(fit.h*(hi*1e12)-fit.work)/charge}};
  r.plots.push_back(std::move(p));
  r.note+="Freier h-Fit mit fitPlanck (2..10 Punkte); Grenzwellenlaenge c/f_G mit gefittetem h, nicht festem Literatur-h. Diagramm in THz und V/eV. ";
  return finish(r,result,error);
}
bool analysisCalculate(const std::string& id,const std::vector<double>& values,NativeResult& result,std::string& error){
  result={};error.clear();const Entry* entry=nullptr;for(const auto& e:analysisEntries)if(id==e.id){entry=&e;break;}
  if(!entry)return bad(error,"Unbekannte Analyse-ID");
  const Kind k=entry->kind;
#ifdef PHYSIK_DATA_CATALOG
  constexpr unsigned bases[]={3,3,3,4,1,1,2,2,1,1,2,2,2,1,1,5,7,1,1,1,1,5,6,7,7,6};
  const size_t base=bases[k];
  const unsigned maxCount=k<=planckLE?(k>=planckFU?10:40):0;
  const unsigned repeated=(k==em?3:k==millikanM||k==millikanR?4:2);
#else
  const Workflow w=workflow(*entry);const size_t base=w.calculation.parameters.size();
  const unsigned maxCount=w.maxCount,repeated=w.repeated.size();
#endif
  for(double value:values)if(!std::isfinite(value))return bad(error,"Nur endliche Eingabewerte erlaubt");
  size_t expected=base;
  if(maxCount){if(values.empty()||values[0]<2||values[0]>maxCount||values[0]!=std::floor(values[0]))
      return bad(error,"Ungueltige Punktzahl (ganzzahlig innerhalb der Grenzen)");
    expected+=static_cast<size_t>(values[0])*repeated;}
  if(values.size()!=expected)return bad(error,"Falsche Anzahl Eingabewerte fuer diese Analyse");
  NativeResult r;bool ok=false;
  if(k<=flux)ok=graphAnalysis(k,values,base,r,error);
  else if(k<=moseley)ok=regression(k,values,base,r,error);
  else if(k<=millikanR)ok=millikan(k,values,base,r,error);
  else if(k<=planckLE){Points p;for(size_t i=0;i<static_cast<size_t>(values[0]);++i){double x=values[base+2*i],y=values[base+2*i+1];
      if(k==planckFE)y/=charge;
      if(k==planckLU)y*=charge;
      p.emplace_back(x,y);}
    ok=planckAnalysis(p,k==planckLU||k==planckLE,r,error);
  }else if(k<=seriesRLC)ok=ac(k,values,r,error);
  else if(k<=freeRLC)ok=oscillation(k,values,r,error);
  else ok=driven(values,r,error);
  return ok&&finish(r,result,error);
}
} // namespace physik

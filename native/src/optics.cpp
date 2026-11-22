// SPDX-License-Identifier: GPL-3.0-or-later
#include "extensions.hpp"
#include <cmath>
#include <initializer_list>
#include <limits>

namespace physik {
namespace {
constexpr double halfPi=pi/2,integerLimit=4503599627370495.0;
enum Op {
  Angle,Lambda,Spacing,Order,Screen,LambdaScreen,SpacingScreen,OrderScreen,
  ScreenLength,MaxOrders,Separation,WaveF,WaveL,WaveV,
  SnellB,SnellA,SnellN1,SnellN2,Critical,Tir,Brewster,BrewsterN1,BrewsterN2,
  Malus,MalusAngle,FirstFilter,FiltersP,FiltersU,FilmD,FilmLambda,
  Central,CentralD,CentralLambda,CentralL,Resolving,ResolvingN,ResolvingK,
  Resolution,Rayleigh,RayleighD,RayleighLambda,Electron,ThresholdH,MoseleyZ,MoseleyLambda
};
#ifdef PHYSIK_DATA_CATALOG
struct Input {constexpr Input(const char*,const char*,const char*,const char*){}};
#else
struct Input {const char* name;const char* unit;const char* display;const char* value;};
#endif
struct Spec {
#ifdef PHYSIK_DATA_CATALOG
  const char* id;uint8_t op,flags,arity;bool single;
  constexpr Spec(const char* key,const char* group,const char*,const char*,Op kind,
                 std::initializer_list<Input> args,unsigned f=0):id(key),op(static_cast<uint8_t>(kind)),flags(static_cast<uint8_t>(f)),arity(static_cast<uint8_t>(args.size())),
    single(group[0]=='O'&&group[1]=='_'&&group[2]=='S'&&group[3]=='L'&&group[4]=='I'&&group[5]=='T'&&group[6]==0){}
#else
  const char* id;const char* category;const char* title;const char* formula;
  Op op;std::initializer_list<Input> inputs;
  // Flags: symmetric screen, odd pipe modes, film half-condition and transmission.
  unsigned flags=0;
#endif
};
constexpr Input lam={"lambda","m","nm","589"},dist={"d","m","um","1.666666667"},
  k={"k (ganzzahlig)","","","1"},theta={"theta","rad","deg","20"},
  length={"L","m","m","1.5"},x={"x einseitig","m","cm","56.7"},
  sep={"D symmetrisch (+k bis -k)","m","cm","113.4"},
  slit={"b Spaltbreite","m","um","100"},
  wn={"n Mode (ab 1)","","","1"},wl={"L","m","m","0.65"},
  wf={"f","Hz","Hz","440"},wv={"v","m/s","m/s","343"},
  n1={"n1","","","1"},n2={"n2","","","1.5"},
  alpha={"alpha (zum Lot)","rad","deg","30"},beta={"beta (zum Lot)","rad","deg","20"},
  intensity={"I0","W/m^2","W/m^2","8"},
  axis={"Achse / Polarisation","rad","deg","0"},
  count={"Filterzahl (1..8)","","","3"},
  nf={"n Film","","","1.33"},ns={"n Substrat","","","1"},
  incidence={"alpha (0 = normal, schraeg: s)","rad","deg","0"},
  filmOrder={"m Zweig (ab 0)","","","1"},
  filmThickness={"d Film","m","nm","300"},
  width={"W zentrale Breite (-1 bis +1)","m","mm","15"},
  grooves={"N beleuchtete Spalte (ganz)","","","1000"},
  resolving={"R benoetigt","","","1000"},
  aperture={"D Kreisoeffnung","m","mm","10"},
  filmN0={"n0 Umgebung","","","1"},
  shielding={"sigma Abschirmung (K-alpha: 1)","","","1"};

// Static descriptors keep catalogue creation and public-API arity in agreement.
#ifdef PHYSIK_DATA_CATALOG
constexpr Spec specs[]={
#else
const Spec specs[]={
#endif
 {"O_DG_ANGLE","O_DG","Doppelspalt/Gitter: Winkel","d sin(theta) = k lambda",Angle,{lam,dist,k}},
 {"O_DG_LAMBDA","O_DG","Doppelspalt/Gitter: Wellenlaenge","lambda = d sin(theta)/k",Lambda,{dist,k,theta}},
 {"O_DG_D","O_DG","Doppelspalt/Gitter: Spaltabstand","d = k lambda/sin(theta)",Spacing,{lam,k,theta}},
 {"O_DG_ORDER","O_DG","Doppelspalt/Gitter: ganze Ordnung","k = d sin(theta)/lambda",Order,{{"lambda","m","nm","500"},{"d","m","um","1"},{"theta","rad","deg","30"}}},
 {"O_DG_SCREEN","O_DG","Gitter: einseitige Schirmposition","x = L tan(asin(k lambda/d))",Screen,{lam,dist,k,length}},
 {"O_DG_LAMBDA_SCREEN","O_DG","Gitter: lambda aus einseitigem x","sin(theta) = x/hypot(L,x)",LambdaScreen,{dist,k,length,x}},
 {"O_DG_D_SCREEN","O_DG","Gitter: d aus einseitigem x","d = k lambda hypot(L,x)/x",SpacingScreen,{lam,k,length,x}},
 {"O_DG_ORDER_SCREEN","O_DG","Gitter: Ordnung aus einseitigem x","k = d x/(lambda hypot(L,x))",OrderScreen,{{"lambda","m","nm","600"},{"d","m","um","1"},{"L","m","m","4"},{"x einseitig","m","m","3"}}},
 {"O_DG_L_SCREEN","O_DG","Gitter: L aus einseitigem x","L = x/tan(theta)",ScreenLength,{lam,dist,k,x}},
 {"O_DG_DISTANCE","O_DG","Gitter: symmetrischer Abstand","D = 2 L tan(theta), von -k bis +k",Screen,{lam,dist,k,length},1},
 {"O_DG_LAMBDA_DISTANCE","O_DG","Gitter: lambda aus symmetrischem D","x = D/2 (nicht einseitig)",LambdaScreen,{dist,k,length,sep},1},
 {"O_DG_D_DISTANCE","O_DG","Gitter: d aus symmetrischem D","x = D/2 (nicht einseitig)",SpacingScreen,{lam,k,length,sep},1},
 {"O_DG_ORDER_DISTANCE","O_DG","Gitter: Ordnung aus symmetrischem D","x = D/2 (nicht einseitig)",OrderScreen,{{"lambda","m","nm","600"},{"d","m","um","1"},{"L","m","m","4"},{"D symmetrisch","m","m","6"}},1},
 {"O_DG_L_DISTANCE","O_DG","Gitter: L aus symmetrischem D","L = (D/2)/tan(theta)",ScreenLength,{lam,dist,k,sep},1},
 {"O_DG_MAX","O_DG","Gitter: maximale Ordnungen","k lambda <= d; Schirm: k lambda < d",MaxOrders,{lam,dist}},
 {"O_DG_SEPARATION","O_DG","Gitter: Abstand zweier Spektrallinien","Delta x = abs(x2-x1), gleiche Seite",Separation,{lam,{"lambda2","m","nm","589.6"},dist,k,length}},
 {"O_FF_F","O_WAVE","Saite fest/fest: Frequenz","f_n = n v/(2 L)",WaveF,{wl,wv,wn}},
 {"O_FF_L","O_WAVE","Saite fest/fest: Laenge","L = n v/(2 f)",WaveL,{wf,wv,wn}},
 {"O_FF_V","O_WAVE","Saite fest/fest: Geschwindigkeit","v = 2 L f/n",WaveV,{wl,wf,wn}},
 {"O_OO_F","O_WAVE","Pfeife offen/offen: Frequenz","f_n = n v/(2 L)",WaveF,{wl,wv,wn}},
 {"O_OO_L","O_WAVE","Pfeife offen/offen: Laenge","L = n v/(2 f)",WaveL,{wf,wv,wn}},
 {"O_OO_V","O_WAVE","Pfeife offen/offen: Geschwindigkeit","v = 2 L f/n",WaveV,{wl,wf,wn}},
 {"O_CO_F","O_WAVE","Pfeife geschlossen/offen: Frequenz","f_n = (2n-1) v/(4 L); n ab 1",WaveF,{wl,wv,wn},2},
 {"O_CO_L","O_WAVE","Pfeife geschlossen/offen: Laenge","L = (2n-1) v/(4 f)",WaveL,{wf,wv,wn},2},
 {"O_CO_V","O_WAVE","Pfeife geschlossen/offen: Geschwindigkeit","v = 4 L f/(2n-1)",WaveV,{wl,wf,wn},2},
 {"O_SNELL_BETA","O_REFRACT","Snell: Brechungswinkel","n1 sin(alpha) = n2 sin(beta)",SnellB,{n1,n2,alpha}},
 {"O_SNELL_ALPHA","O_REFRACT","Snell: Einfallswinkel","alpha = asin(n2 sin(beta)/n1)",SnellA,{n1,n2,beta}},
 {"O_SNELL_N1","O_REFRACT","Snell: Index n1","n1 = n2 sin(beta)/sin(alpha)",SnellN1,{n2,alpha,beta}},
 {"O_SNELL_N2","O_REFRACT","Snell: Index n2","n2 = n1 sin(alpha)/sin(beta)",SnellN2,{n1,alpha,beta}},
 {"O_CRITICAL","O_REFRACT","Grenzwinkel","alpha_c = asin(n2/n1), n1 > n2",Critical,{{"n1","","","1.5"},{"n2","","","1"}}},
 {"O_TIR","O_REFRACT","Brechung oder Totalreflexion","TIR genau fuer n1 sin(alpha) > n2",Tir,{n1,n2,alpha}},
 {"O_BREWSTER","O_REFRACT","Brewster-Winkel","tan(alpha_B) = n2/n1",Brewster,{n1,n2}},
 {"O_BREWSTER_N1","O_REFRACT","Brewster: Index n1","n1 = n2/tan(alpha_B)",BrewsterN1,{n2,alpha}},
 {"O_BREWSTER_N2","O_REFRACT","Brewster: Index n2","n2 = n1 tan(alpha_B)",BrewsterN2,{n1,alpha}},
 {"O_MALUS","O_POLAR","Malus: polarisiertes Licht","I = I0 cos(alpha)^2",Malus,{intensity,{"alpha zwischen Achsen","rad","deg","30"}}},
 {"O_MALUS_ANGLE","O_POLAR","Malus: Hauptwinkel 0..90 Grad","alpha = acos(sqrt(I/I0))",MalusAngle,{intensity,{"I","W/m^2","W/m^2","2"}}},
 {"O_FIRST_FILTER","O_POLAR","Unpolarisiert: erster idealer Filter","I = I0/2",FirstFilter,{intensity}},
 {"O_FILTERS_P","O_POLAR","Filterfolge: polarisiert","I_j = I_(j-1) cos(A_j-A_(j-1))^2",FiltersP,{count,intensity,axis}},
 {"O_FILTERS_U","O_POLAR","Filterfolge: unpolarisiert","Erster Filter halbiert; danach Malus",FiltersU,{count,intensity}},
 {"O_FILM_RMAX_D","O_FILM","Film: Dicke fuer Reflexionsmaximum","2 n_f d cos(beta) = (m+q) lambda",FilmD,{filmN0,nf,ns,incidence,lam,filmOrder}},
 {"O_FILM_RMIN_D","O_FILM","Film: Dicke fuer Reflexionsminimum","q aus automatischen Reflexionsphasen",FilmD,{filmN0,nf,ns,incidence,lam,filmOrder},4},
 {"O_FILM_TMAX_D","O_FILM","Film: Dicke fuer Transmissionsmaximum","T-Maximum entspricht R-Minimum (verlustfrei)",FilmD,{filmN0,nf,ns,incidence,lam,filmOrder},12},
 {"O_FILM_TMIN_D","O_FILM","Film: Dicke fuer Transmissionsminimum","T-Minimum entspricht R-Maximum (verlustfrei)",FilmD,{filmN0,nf,ns,incidence,lam,filmOrder},8},
 {"O_FILM_RMAX_LAMBDA","O_FILM","Film: lambda fuer Reflexionsmaximum","lambda = 2 n_f d cos(beta)/(m+q)",FilmLambda,{filmN0,nf,ns,incidence,filmThickness,filmOrder}},
 {"O_FILM_RMIN_LAMBDA","O_FILM","Film: lambda fuer Reflexionsminimum","m+q = 0 identifiziert keine Wellenlaenge",FilmLambda,{filmN0,nf,ns,incidence,filmThickness,filmOrder},4},
 {"O_FILM_TMAX_LAMBDA","O_FILM","Film: lambda fuer Transmissionsmaximum","T-Maximum entspricht R-Minimum (verlustfrei)",FilmLambda,{filmN0,nf,ns,incidence,filmThickness,filmOrder},12},
 {"O_FILM_TMIN_LAMBDA","O_FILM","Film: lambda fuer Transmissionsminimum","T-Minimum entspricht R-Maximum (verlustfrei)",FilmLambda,{filmN0,nf,ns,incidence,filmThickness,filmOrder},8},
 {"O_SS_ANGLE","O_SLIT","Einzelspalt: Minimumwinkel","b sin(theta) = k lambda; k >= 1",Angle,{lam,slit,k}},
 {"O_SS_LAMBDA","O_SLIT","Einzelspalt: Wellenlaenge","lambda = b sin(theta)/k",Lambda,{slit,k,theta}},
 {"O_SS_B","O_SLIT","Einzelspalt: Spaltbreite","b = k lambda/sin(theta)",Spacing,{lam,k,theta}},
 {"O_SS_ORDER","O_SLIT","Einzelspalt: Minimumordnung","k = b sin(theta)/lambda; k >= 1",Order,{{"lambda","m","nm","500"},{"b","m","um","1"},{"theta","rad","deg","30"}}},
 {"O_SS_SCREEN","O_SLIT","Einzelspalt: Minimum auf Schirm","x_k = L tan(asin(k lambda/b))",Screen,{lam,slit,k,length}},
 {"O_SS_LAMBDA_SCREEN","O_SLIT","Einzelspalt: lambda aus Minimum","lambda = b x/(k hypot(L,x))",LambdaScreen,{slit,k,length,x}},
 {"O_SS_B_SCREEN","O_SLIT","Einzelspalt: b aus Minimum","b = k lambda hypot(L,x)/x",SpacingScreen,{lam,k,length,x}},
 {"O_SS_ORDER_SCREEN","O_SLIT","Einzelspalt: Ordnung aus Minimum","k = b x/(lambda hypot(L,x))",OrderScreen,{{"lambda","m","nm","600"},{"b","m","um","1"},{"L","m","m","4"},{"x einseitig","m","m","3"}}},
 {"O_SS_L_SCREEN","O_SLIT","Einzelspalt: Schirmabstand","L = x_k/tan(theta_k)",ScreenLength,{lam,slit,k,x}},
 {"O_SS_CENTRAL","O_SLIT","Einzelspalt: zentrale volle Breite","W = 2 L tan(asin(lambda/b))",Central,{lam,slit,length}},
 {"O_SS_CENTRAL_B","O_SLIT","Zentrale Breite: Spaltbreite","b = lambda hypot(L,W/2)/(W/2)",CentralD,{lam,length,width}},
 {"O_SS_CENTRAL_LAMBDA","O_SLIT","Zentrale Breite: Wellenlaenge","lambda = b (W/2)/hypot(L,W/2)",CentralLambda,{slit,length,width}},
 {"O_SS_CENTRAL_L","O_SLIT","Zentrale Breite: Schirmabstand","L = (W/2)/tan(asin(lambda/b))",CentralL,{lam,slit,width}},
 {"O_GRATING_R","O_RESOLVE","Gitter: Aufloesungsvermoegen","R = k N (ideales Gitter)",Resolving,{k,grooves}},
 {"O_GRATING_N","O_RESOLVE","Gitter: minimale beleuchtete Spaltzahl","N_min = ceil(R/k)",ResolvingN,{resolving,k}},
 {"O_GRATING_K","O_RESOLVE","Gitter: minimale Beugungsordnung","k_min = ceil(R/N)",ResolvingK,{resolving,grooves}},
 {"O_GRATING_RESOLUTION","O_RESOLVE","Gitter: trennbarer Wellenlaengenabstand","Delta lambda = lambda/(k N)",Resolution,{lam,k,grooves}},
 {"O_RAYLEIGH","O_RESOLVE","Kreisoeffnung: Rayleigh-Winkel","sin(theta) = 1.22 lambda/D",Rayleigh,{lam,aperture}},
 {"O_RAYLEIGH_D","O_RESOLVE","Rayleigh: Oeffnungsdurchmesser","D = 1.22 lambda/sin(theta)",RayleighD,{lam,theta}},
 {"O_RAYLEIGH_LAMBDA","O_RESOLVE","Rayleigh: Wellenlaenge","lambda = D sin(theta)/1.22",RayleighLambda,{aperture,theta}},
 {"O_ELECTRON_COMPARE","O_QUANTUM","Elektron: klassisch vs relativistisch","lambda_rel = lambda_klass/sqrt(1+eU/(2mc^2))",Electron,{{"U","V","kV","50"}}},
 {"O_THRESHOLD_H","O_QUANTUM","Planck h aus Bragg-Einsatzwinkel","h = 2 e d U sin(theta)/(k c)",ThresholdH,{{"U","V","kV","30"},{"d Netzebenen","m","pm","201"},{"theta Glanzwinkel","rad","deg","5.9"},k}},
 {"O_MOSELEY_Z","O_QUANTUM","Moseley K-alpha: Kernladungszahl","hc/lambda = (3/4) 13.6 eV (Z-sigma)^2",MoseleyZ,{{"lambda K-alpha","m","pm","154"},shielding}},
 {"O_MOSELEY_LAMBDA","O_QUANTUM","Moseley K-alpha: Wellenlaenge","lambda = hc/((3/4) 13.6 eV (Z-sigma)^2)",MoseleyLambda,{{"Z (ganz, 2..118)","","","29"},shielding}}
};
bool natural(double v,double minimum=1){return v>=minimum&&v<=integerLimit&&v==std::floor(v);}
bool angle(double v,bool grazing=false){return v>=0&&(grazing?v<=halfPi:v<halfPi);}
bool axisAngle(double v){return v>=-pi&&v<=pi;}
bool positive(double v){return v>0&&std::isfinite(v);}
double cosine2(double a){
  // Exact orthogonal axes must give exact darkness, not a cos(pi/2) residue.
  const double r=std::remainder(a,pi);
  return std::abs(r)==halfPi?0:std::cos(r)*std::cos(r);
}
bool integerOrder(double raw,double minimum,double& result){
  result=std::round(raw);
  return std::isfinite(raw)&&natural(result,minimum)&&(result!=0||raw==0)&&
    std::abs(raw-result)<=32*std::numeric_limits<double>::epsilon()*std::fmax(1.0,std::abs(raw));
}
bool dynamic(const Spec& s){return s.op==FiltersP||s.op==FiltersU;}
size_t inputCount(const Spec& s){
#ifdef PHYSIK_DATA_CATALOG
  return s.arity;
#else
  return s.inputs.size();
#endif
}
#ifndef PHYSIK_DATA_CATALOG
Workflow workflow(const Spec& s){
  Workflow w;
  w.calculation={s.title,s.formula,"Ergebnis","",s.id,{}};
  for(const auto& i:s.inputs){w.calculation.parameters.push_back({i.name,i.unit,i.display});w.defaults.push_back(i.value);}
  if(dynamic(s)){w.minCount=1;w.maxCount=8;w.repeated={{"Filterachse","rad","deg"}};w.repeatedDefaults={"0"};}
  return w;
}
#endif
}

std::vector<NativeCategory> opticsCategories(){
  return {{"O_DG","Doppelspalt und Gitter (exakt)"},{"O_WAVE","Stehende Wellen"},
    {"O_REFRACT","Brechung und Brewster"},{"O_POLAR","Polarisation und Filter"},
    {"O_FILM","Duenne Schichten"},{"O_SLIT","Einzelspalt"},
    {"O_RESOLVE","Optische Aufloesung"},{"O_QUANTUM","Quanten: gezielte Ergaenzungen"}};
}
#ifndef PHYSIK_DATA_CATALOG
std::vector<Workflow> opticsCatalog(const std::string& category){
  std::vector<Workflow> result;
  for(const auto& s:specs)if(category==s.category)result.push_back(workflow(s));
  return result;
}
#endif

bool opticsCalculate(const std::string& id,const std::vector<double>& v,NativeResult& result,std::string& error){
  result={};error.clear();
  const auto bad=[&](const char* message){error=message;return false;};
  const Spec* selected=nullptr;
  for(const auto& s:specs)if(id==s.id){selected=&s;break;}
  if(!selected)return bad("Unbekannte Optik-Rechnung");
  const Spec& s=*selected;
  if(v.size()<inputCount(s))return bad("Falsche Anzahl Eingaben");
  for(double a:v)if(!std::isfinite(a))return bad("Alle Eingaben muessen endlich sein");
  if(dynamic(s)){
    if(!natural(v[0])||v[0]>8)return bad("Filterzahl muss ganzzahlig zwischen 1 und 8 sein");
    if(v.size()!=inputCount(s)+static_cast<unsigned>(v[0]))return bad("Filterzahl und Eingaben passen nicht zusammen");
  }else if(v.size()!=inputCount(s))return bad("Falsche Anzahl Eingaben");
  NativeResult out;
  const auto add=[&](const char* name,double value,const char* unit){out.values.push_back({name,value,unit});};
  const auto needPositive=[&](double value,const char* name,const char* unit){
    if(!positive(value))return false;
    add(name,value,unit);return true;
  };
#ifdef PHYSIK_DATA_CATALOG
  const bool single=s.single;
#else
  const bool single=std::string(s.category)=="O_SLIT";
#endif
  const double minimum=single?1:0;
  switch(s.op){
  case Angle:case Lambda:case Spacing:case Order:case Screen:case LambdaScreen:
  case SpacingScreen:case OrderScreen:case ScreenLength:{
    double wavelength=0,d=0,order=0,sine=0,L=0,position=0;
    const bool fromScreen=s.op==LambdaScreen||s.op==SpacingScreen||s.op==OrderScreen;
    if(fromScreen){
      L=v[2];position=v[3]/((s.flags&1)?2:1);
      if(!positive(L)||position<0)return bad("Schirm: L > 0 und Abstand >= 0 erforderlich");
      sine=position/std::hypot(L,position);
      if(!angle(std::atan2(position,L))||sine>=1||(position>0&&sine==0))
        return bad("Schirmwinkel ausserhalb des endlichen Zahlenbereichs unter 90 Grad");
    }
    if(s.op==Lambda||s.op==Spacing){
      if(!angle(v[2],true)||v[2]==0)return bad("Umkehrung: Winkel groesser 0 bis 90 Grad erforderlich");
      sine=std::sin(v[2]);
    }
    if(s.op==Lambda||s.op==LambdaScreen){
      d=v[0];order=v[1];
      if(!positive(d)||!natural(order)||sine<=0)return bad("lambda ist nur fuer d > 0, k >= 1 und Winkel > 0 eindeutig");
      wavelength=d*(sine/order);
    }else if(s.op==Spacing||s.op==SpacingScreen){
      wavelength=v[0];order=v[1];
      if(!positive(wavelength)||!natural(order)||sine<=0)return bad("Spaltbreite: lambda > 0, k >= 1 und Winkel > 0 erforderlich");
      d=wavelength*(order/sine);
    }else{
      wavelength=v[0];d=v[1];
      if(!positive(wavelength)||!positive(d))return bad("Wellenlaenge und Spaltabstand muessen positiv sein");
      if(s.op==Order||s.op==OrderScreen){
        if(s.op==Order){if(!angle(v[2],true))return bad("Winkel muss zwischen 0 und 90 Grad liegen");sine=std::sin(v[2]);}
        if(!integerOrder((d/wavelength)*sine,minimum,order))return bad("Kein Maximum/Minimum ganzzahliger Ordnung an dieser Stelle");
      }else{
        order=v[2];
        if(!natural(order,minimum))return bad("Ordnung muss ganzzahlig sein (Einzelspalt ab 1)");
        sine=(order*wavelength)/d;
      }
    }
    if(!positive(wavelength)||!positive(d)||!std::isfinite(sine)||sine<0||sine>1||(order>0&&sine==0))
      return bad("Keine propagierende Ordnung: k lambda darf d nicht uebersteigen");
    const double a=std::asin(sine);
    if(s.op==Lambda||s.op==LambdaScreen)add("lambda",wavelength,"m");
    if(s.op==Spacing||s.op==SpacingScreen)add(single?"b":"d",d,"m");
    if(s.op==Order||s.op==OrderScreen)add("k",order,"");
    add("theta",a,"rad");
    if(s.op==Screen||s.op==ScreenLength){
      if(sine>=1)return bad("Streifende Ordnung bei 90 Grad: keine endliche Schirmposition");
      const double tangent=sine/std::sqrt((1-sine)*(1+sine));
      if(s.op==Screen){
        L=v[3];if(!positive(L))return bad("Schirmabstand L muss positiv sein");
        const double xx=L*tangent;
        if(order>0&&!positive(xx))return bad("Schirmposition ausserhalb des Zahlenbereichs");
        add("x einseitig",xx,"m");add("D symmetrisch",2*xx,"m");
      }else{
        position=v[3]/((s.flags&1)?2:1);
        if(!positive(position)||sine<=0)return bad("L ist nur fuer Abstand > 0 und k >= 1 eindeutig");
        if(!needPositive(position/tangent,"L","m"))return bad("Schirmabstand ausserhalb des Zahlenbereichs");
      }
    }
    if(sine==1)out.note="Streifende Ordnung (90 Grad): propagierend, aber keine endliche Position auf einem ebenen Schirm.";
    else out.note=single?"Fraunhofer-Einzelspalt; k bezeichnet ein Minimum, die Mitte ist kein Minimum.":
      "Fraunhofer, senkrechter Einfall; theta >= 0 bezeichnet eine Seite, -theta die gespiegelte Ordnung. Keine Kleinwinkelnaherung.";
    break;
  }
  case MaxOrders:{
    if(!positive(v[0])||!positive(v[1]))return bad("lambda und d muessen positiv sein");
    const double ratio=v[1]/v[0];
    if(!positive(ratio)||ratio>integerLimit)return bad("Ordnungszahl ausserhalb des exakten Ganzzahlbereichs");
    double max=std::floor(ratio);
    if(max*v[0]>v[1])--max;
    if(max<integerLimit&&(max+1)*v[0]<=v[1])++max;
    const double screen=max*v[0]==v[1]?max-1:max;
    add("k_max propagierend",max,"");add("k_max endlicher Schirm",screen,"");
    add("Maxima gesamt inkl. Mitte",2*max+1,"");
    add("Maxima auf endlichem Schirm",2*screen+1,"");
    out.note="Beide Seiten mitgezaehlt; bei k lambda = d ist die Ordnung streifend und fehlt auf jedem endlichen ebenen Schirm.";
    break;
  }
  case Separation:{
    if(!positive(v[0])||!positive(v[1])||!positive(v[2])||!natural(v[3])||!positive(v[4]))
      return bad("Spektrallinien: lambda1, lambda2, d, L > 0 und ganze Ordnung k >= 1 erforderlich");
    const double a=(v[3]*v[0])/v[2],b=(v[3]*v[1])/v[2];
    if(a>=1||b>=1||!std::isfinite(a)||!std::isfinite(b))return bad("Beide Spektrallinien brauchen einen endlichen Schirmwinkel unter 90 Grad");
    const double x1=v[4]*a/std::sqrt((1-a)*(1+a)),x2=v[4]*b/std::sqrt((1-b)*(1+b));
    add("x1 einseitig",x1,"m");add("x2 einseitig",x2,"m");add("Linienabstand",std::abs(x2-x1),"m");
    out.note="Zwei Wellenlaengen derselben Ordnung auf derselben Seite; kein Abstand zwischen +k und -k.";
    break;
  }
  case WaveF:case WaveL:case WaveV:{
    if(!positive(v[0])||!positive(v[1])||!natural(v[2]))return bad("Stehende Welle: L, f, v > 0 und ganze Modenzahl n >= 1 erforderlich");
    const double harmonic=(s.flags&2)?2*v[2]-1:v[2],den=(s.flags&2)?4:2;
    double L=0,f=0,speed=0;
    if(s.op==WaveF){L=v[0];speed=v[1];f=(speed/L)*(harmonic/den);}
    else if(s.op==WaveL){f=v[0];speed=v[1];L=(speed/f)*(harmonic/den);}
    else {L=v[0];f=v[1];speed=(f/harmonic)*(den*L);}
    if(!needPositive(L,"L","m")||!needPositive(f,"f","Hz")||!needPositive(speed,"v","m/s")||
       !needPositive(speed/f,"lambda","m"))return bad("Stehende Welle ausserhalb des Zahlenbereichs");
    add("Harmonische",harmonic,"");
    out.note=(s.flags&2)?"Geschlossen/offen: Modenzahl n = 1,2,... entspricht den Harmonischen 1,3,5,...; ohne Endkorrektur.":
      "Fest/fest: Schwingungsknoten an beiden Enden. Offen/offen: Schwingungsbaeuche (Druckknoten); ohne Endkorrektur.";
    break;
  }
  case SnellB:case SnellA:case SnellN1:case SnellN2:case Critical:case Tir:{
    if(!positive(v[0]))return bad("Brechungsindizes muessen positiv sein");
    if(s.op==SnellN1||s.op==SnellN2){
      if(!angle(v[1])||!angle(v[2],true)||v[1]==0||v[2]==0)
        return bad("Indexumkehrung: 0 < alpha < 90 Grad, 0 < beta <= 90 Grad erforderlich");
      const double value=s.op==SnellN1?v[0]*(std::sin(v[2])/std::sin(v[1])):
        v[0]*(std::sin(v[1])/std::sin(v[2]));
      if(!needPositive(value,s.op==SnellN1?"n1":"n2",""))return bad("Brechungsindex ausserhalb des Zahlenbereichs");
      break;
    }
    if(!positive(v[1]))return bad("Brechungsindizes muessen positiv sein");
    if(s.op==Critical){
      if(v[0]<=v[1])return bad("Grenzwinkel nur beim Uebergang n1 > n2");
      add("alpha kritisch",std::asin(v[1]/v[0]),"rad");break;
    }
    if(!angle(v[2],s.op==SnellA))return bad("Einfallswinkel: 0 bis unter 90 Grad; Brechungswinkel bis 90 Grad");
    double ratio=s.op==SnellA?(v[1]/v[0])*std::sin(v[2]):(v[0]/v[1])*std::sin(v[2]);
    // Compare angles at the boundary: sin(asin(n2/n1))*n1/n2 can round above 1.
    const double critical=v[0]>v[1]?std::asin(v[1]/v[0]):halfPi;
    const bool total=s.op!=SnellA&&v[2]>critical;
    if(s.op!=SnellA&&v[2]>0&&v[2]==critical)ratio=1;
    if(v[2]==0)ratio=0;
    if(s.op==SnellA&&v[1]>=v[0]&&v[2]>=std::asin(v[0]/v[1]))
      return bad("Umkehrung erfordert unmoeglichen oder streifenden Einfall");
    if(!std::isfinite(ratio))return bad("Brechung ausserhalb des Zahlenbereichs");
    if(s.op!=SnellA&&!total&&ratio>1)ratio=1;
    if(s.op==Tir){
      add("Totalreflexion",total?1:0,"");
      if(v[0]>v[1])add("alpha kritisch",critical,"rad");
      if(total){out.note="Totalreflexion: kein reeller gebrochener Strahl. Am Grenzwinkel selbst verlaeuft der Strahl streifend.";break;}
    }else if(total||ratio>1)return bad("Kein reeller Winkel (Totalreflexion oder unmoegliche Umkehrung)");
    if(ratio>1)return bad("Brechungswinkel numerisch nicht aufloesbar");
    const double a=std::asin(ratio);
    if(s.op==SnellA&&!angle(a))return bad("Umkehrung ergibt streifenden Einfall; alpha muss unter 90 Grad liegen");
    add(s.op==SnellA?"alpha":"beta",a,"rad");
    out.note=(s.op!=SnellA&&v[2]>0&&v[2]==critical)?
      "Grenzwinkel: gebrochener Strahl bei 90 Grad, noch keine Totalreflexion.":
      "Winkel zum Lot; transparente Medien mit positiven Brechungsindizes.";
    break;
  }
  case Brewster:case BrewsterN1:case BrewsterN2:{
    if(!positive(v[0])||(s.op==Brewster?!positive(v[1]):(!angle(v[1])||v[1]==0)))
      return bad("Brewster: Indizes > 0 und Winkel strikt zwischen 0 und 90 Grad");
    if(s.op!=Brewster&&v[1]==pi/4)return bad("45 Grad bedeutet indexgleiche Medien: kein polarisierender Brewster-Effekt");
    if(s.op==Brewster){
      if(v[0]==v[1])return bad("Gleiche Medien reflektieren nicht: kein polarisierender Brewster-Effekt");
      const double a=std::atan2(v[1],v[0]);
      if(!angle(a)||a==0)return bad("Brewster-Winkel ausserhalb des Zahlenbereichs");
      add("alpha Brewster",a,"rad");add("beta",halfPi-a,"rad");
    }else{
      const double value=s.op==BrewsterN1?v[0]/std::tan(v[1]):v[0]*std::tan(v[1]);
      if(value==v[0])return bad("Gleiche Medien: kein polarisierender Brewster-Effekt");
      if(!needPositive(value,s.op==BrewsterN1?"n1":"n2",""))return bad("Brechungsindex ausserhalb des Zahlenbereichs");
    }
    out.note="Ideale nichtmagnetische, nichtabsorbierende Medien; reflektiertes Licht ist s-polarisiert.";
    break;
  }
  case Malus:case MalusAngle:case FirstFilter:{
    if(v[0]<0)return bad("Intensitaet darf nicht negativ sein");
    if(s.op==FirstFilter)add("I",v[0]/2,"W/m^2");
    else if(s.op==Malus){
      if(!axisAngle(v[1]))return bad("Relativen Filterwinkel zwischen -180 und 180 Grad angeben");
      add("I",v[0]*cosine2(v[1]),"W/m^2");
    }else{
      if(v[0]<=0||v[1]<0||v[1]>v[0])return bad("Winkel nur fuer I0 > 0 und 0 <= I <= I0 eindeutig");
      add("alpha",std::acos(std::sqrt(v[1]/v[0])),"rad");
      out.note="Hauptzweig 0..90 Grad; weitere periodische/spiegelbildliche Filterrichtungen sind gleichwertig.";
    }
    break;
  }
  case FiltersP:case FiltersU:{
    if(v[1]<0)return bad("Intensitaet darf nicht negativ sein");
    const bool polarized=s.op==FiltersP;const unsigned n=static_cast<unsigned>(v[0]),base=polarized?3:2;
    for(unsigned j=polarized?2:base;j<v.size();++j)if(!axisAngle(v[j]))return bad("Filterachsen zwischen -180 und 180 Grad angeben");
    double I=v[1],previous=polarized?v[2]:v[base];
    Plot plot{"Intensitaet nach Filter","Filter","","I","W/m^2",{{0,I}},{},true};
    for(unsigned j=0;j<n;++j){
      I*=(!polarized&&j==0)?.5:cosine2(v[base+j]-previous);previous=v[base+j];
      out.values.push_back({"I nach Filter "+std::to_string(j+1),I,"W/m^2"});
      plot.points.push_back({j+1,I});
    }
    add("I",I,"W/m^2");out.plots.push_back(plot);
    out.note="Ideale verlustfreie Polarisatoren; Achsen sind absolute Richtungen, nicht jeweils relative Drehwinkel.";
    break;
  }
  case FilmD:case FilmLambda:{
    if(!positive(v[0])||!positive(v[1])||!positive(v[2])||!angle(v[3])||v[4]<0||!natural(v[5],0))
      return bad("Film: Indizes > 0, 0 <= alpha < 90 Grad, Groesse >= 0 und ganze Ordnung m >= 0");
    if(v[0]==v[1]||v[1]==v[2])return bad("Indexgleiche Grenzflaeche: keine Zwei-Strahl-Interferenz");
    const double tangential=v[0]*std::sin(v[3]),filmSine=tangential/v[1];
    const bool limiting=v[3]>0&&((v[0]>v[1]&&v[3]>=std::asin(v[1]/v[0]))||
      (v[0]>v[2]&&v[3]>=std::asin(v[2]/v[0])));
    if(limiting||!std::isfinite(tangential)||filmSine>=1||tangential/v[2]>=1)
      return bad("Film/Substrat streifend oder totalreflektierend: 0/pi-Phasenmodell nicht anwendbar");
    const double cosFilm=std::sqrt((1-filmSine)*(1+filmSine));
    const bool front=v[1]>v[0],back=v[2]>v[1],mismatch=front!=back;
    const double offset=(mismatch!=static_cast<bool>(s.flags&4))?.5:0;
    const double branch=v[5]+offset,scale=2*v[1]*cosFilm;
    double d=0,wavelength=0;
    if(s.op==FilmD){
      wavelength=v[4];if(!positive(wavelength))return bad("Wellenlaenge muss positiv sein");
      d=(wavelength/scale)*branch;
      if(branch>0&&!positive(d))return bad("Filmdicke ausserhalb des Zahlenbereichs");
      add("d",d,"m");
    }else{
      d=v[4];if(!positive(d)||branch==0)return bad("Nullzweig m+q = 0 oder d = 0 bestimmt keine eindeutige Wellenlaenge");
      wavelength=(d/branch)*scale;
      if(!needPositive(wavelength,"lambda","m"))return bad("Wellenlaenge ausserhalb des Zahlenbereichs");
    }
    add("beta Film",std::asin(filmSine),"rad");add("Phase vorn",front?pi:0,"rad");
    add("Phase hinten",back?pi:0,"rad");add("Gangunterschied",scale*d,"m");add("m+q",branch,"");
    out.note="Phasenbedingungen, keine absolute Intensitaet. Transparenter verlustfreier Film, schraeg fuer s-Polarisation; Snell-Winkel im Film. m zaehlt nichtnegative Zweige; T-Extrema komplementaer zu R.";
    if(branch==0)out.note+=" Nullzweig: d = 0 (keine endliche Schicht), keine spektrale Farbauswahl.";
    break;
  }
  case Central:case CentralD:case CentralLambda:case CentralL:{
    if(!positive(v[0])||!positive(v[1])||!positive(v[2]))return bad("Zentrale Breite: alle Groessen muessen positiv sein");
    double value=0;
    if(s.op==CentralD||s.op==CentralLambda){
      const double xx=v[2]/2,sine=xx/std::hypot(v[1],xx);
      if(sine<=0||sine>=1)return bad("Zentrale Breite ausserhalb des endlichen Schirmbereichs");
      value=s.op==CentralD?v[0]/sine:v[0]*sine;
    }else{
      const double sine=v[0]/v[1];
      if(sine<=0||sine>=1)return bad("Erstes Minimum nicht auf endlichem Schirm (lambda muss b unterschreiten)");
      const double tangent=sine/std::sqrt((1-sine)*(1+sine));
      value=s.op==Central?2*v[2]*tangent:(v[2]/2)/tangent;
    }
    if(!needPositive(value,s.op==Central?"W":s.op==CentralD?"b":s.op==CentralLambda?"lambda":"L","m"))
      return bad("Zentrale Breite ausserhalb des Zahlenbereichs");
    out.note="Volle Breite des zentralen Hauptmaximums zwischen den Minima -1 und +1, nicht Halbbreite.";
    break;
  }
  case Resolving:case ResolvingN:case ResolvingK:case Resolution:{
    double order=0,N=0;
    if(s.op==Resolution){
      if(!positive(v[0])||!natural(v[1])||!natural(v[2]))return bad("Aufloesung: lambda > 0, k und N ganzzahlig >= 1");
      order=v[1];N=v[2];
    }else if(s.op==Resolving){
      if(!natural(v[0])||!natural(v[1]))return bad("k und N muessen ganzzahlig >= 1 sein");
      order=v[0];N=v[1];
    }else{
      if(!positive(v[0])||!natural(v[1]))return bad("R > 0 und k bzw. N ganzzahlig >= 1 erforderlich");
      const double needed=std::ceil(v[0]/v[1]);
      if(!natural(needed))return bad("Benoetigte Spaltzahl/Ordnung ausserhalb des Ganzzahlbereichs");
      order=s.op==ResolvingK?needed:v[1];N=s.op==ResolvingN?needed:v[1];
      add(s.op==ResolvingN?"N minimal":"k minimal",needed,"");
    }
    const double R=order*N;
    if(!positive(R)||R>integerLimit)return bad("Aufloesung ausserhalb des exakten Ganzzahlbereichs");
    add("R",R,"");
    if(s.op==Resolution&&!needPositive(v[0]/R,"Delta lambda","m"))return bad("Wellenlaengenabstand zu klein");
    out.note="Ideales beleuchtetes Gitter: R = k N. Existenz der Ordnung zusaetzlich mit k lambda <= d pruefen.";
    break;
  }
  case Rayleigh:case RayleighD:case RayleighLambda:{
    if(!positive(v[0]))return bad("Wellenlaenge/Durchmesser muss positiv sein");
    if(s.op==Rayleigh){
      if(!positive(v[1]))return bad("Durchmesser muss positiv sein");
      const double sine=1.22*(v[0]/v[1]);
      if(sine<=0||sine>1)return bad("Kein propagierendes Rayleigh-Minimum (1.22 lambda > D)");
      add("theta Rayleigh",std::asin(sine),"rad");
    }else{
      if(!angle(v[1],true)||v[1]==0)return bad("Rayleigh-Umkehrung: 0 < theta <= 90 Grad erforderlich");
      const double value=s.op==RayleighD?1.22*(v[0]/std::sin(v[1])):v[0]*(std::sin(v[1])/1.22);
      if(!needPositive(value,s.op==RayleighD?"D":"lambda","m"))return bad("Rayleigh-Wert ausserhalb des Zahlenbereichs");
    }
    out.note="Ideale Kreisoeffnung, Fraunhofer/Airy mit Faktor 1.22; kleine Winkel: theta etwa 1.22 lambda/D. Keine Aussage ueber reale Aberrationen.";
    break;
  }
  case Electron:{
    if(!positive(v[0]))return bad("Beschleunigungsspannung muss positiv sein");
    const double restVoltage=electronMass*light*light/charge,a=v[0]/(2*restVoltage),factor=std::sqrt(1+a);
    const double classical=(planck/std::sqrt(2*electronMass*charge))/std::sqrt(v[0]);
    if(!needPositive(classical,"lambda klassisch","m")||!needPositive(classical/factor,"lambda relativistisch","m"))
      return bad("Elektronenwellenlaenge ausserhalb des Zahlenbereichs");
    add("Relativer Fehler",100*(a/(factor+1)),"%");
    const double u=v[0]/restVoltage,betaClassical=std::sqrt(2*u);
    add("v klassisch",light*betaClassical,"m/s");
    add("v relativistisch",light*std::sqrt((u/(1+u))*(1+1/(1+u))),"m/s");
    add("Klassisch unter 0.1 c",betaClassical<.1?1:0,"");
    out.note="Vergleich der klassischen Naeherung mit relativistischer Rechnung aus Ruhe; Fehler = (klassisch-rel)/rel. Bestehende klassische Rechner behalten ihre 0.1-c-Sperre.";
    if(betaClassical>=.1)out.note+=" WARNUNG: v klassisch >= 0.1 c; klassischer Wert hier nur zum Modellvergleich, relativistische Rechnung verwenden.";
    break;
  }
  case ThresholdH:{
    if(!positive(v[0])||!positive(v[1])||!angle(v[2],true)||v[2]==0||!natural(v[3]))
      return bad("Bragg-Einsatz: U,d > 0; Glanzwinkel 0 < theta <= 90 Grad; ganze Ordnung k >= 1");
    const double wavelength=2*v[1]*(std::sin(v[2])/v[3]);
    if(!needPositive((charge*v[0]/light)*wavelength,"h","J s")||!needPositive(wavelength,"lambda Grenze","m"))
      return bad("Planck-Bestimmung ausserhalb des Zahlenbereichs");
    out.note="theta ist der Glanzwinkel zur Netzebene, NICHT der Detektorwinkel 2 theta. Einsatz der Bremsstrahlung: eU = hc/lambda.";
    break;
  }
  case MoseleyZ:case MoseleyLambda:{
    if(!positive(v[0])||v[1]<0||v[1]>=118)return bad("Moseley: positive Groesse und 0 <= Abschirmung < 118 erforderlich");
    constexpr double energy=.75*13.6*charge;
    if(s.op==MoseleyZ){
      const double Z=v[1]+std::sqrt((planck*light/energy)/v[0]);
      if(!positive(Z)||Z<2||Z>118)return bad("Geschaetzte Kernladungszahl ausserhalb 2..118");
      add("Z geschaetzt",Z,"");add("Z naechste ganze Zahl",std::round(Z),"");
      out.note=std::round(Z)==29?"Naechstes Element: Cu (Kupfer), Z = 29. ":"Z ist eine Modellabschaetzung, kein exakter Elementnachweis. ";
    }else{
      if(!natural(v[0],2)||v[0]>118||v[0]<=v[1])return bad("Z ganzzahlig 2..118 und groesser als Abschirmung erforderlich");
      const double effective=v[0]-v[1];
      if(!needPositive((planck*light/energy)/effective/effective,"lambda K-alpha","m"))return bad("Moseley-Wellenlaenge ausserhalb des Zahlenbereichs");
    }
    out.note+="K-alpha-Modell: (3/4) 13.6 eV (Z-sigma)^2; Abschirmung explizit, Standard sigma = 1. Keine Feinstruktur/relativistische Korrektur.";
    break;
  }
  }
  if(out.values.empty())return bad("Keine Optik-Ergebnisse");
  for(const auto& value:out.values)if(!std::isfinite(value.value))return bad("Ergebnis ausserhalb des endlichen Zahlenbereichs");
  for(const auto& plot:out.plots){
    if(plot.points.size()>400||plot.model.size()>400)return bad("Zu viele Diagrammpunkte");
    for(const auto* data:{&plot.points,&plot.model})for(const auto& p:*data)
      if(!std::isfinite(p.first)||!std::isfinite(p.second))return bad("Diagramm ausserhalb des Zahlenbereichs");
  }
  result=std::move(out);return true;
}
}

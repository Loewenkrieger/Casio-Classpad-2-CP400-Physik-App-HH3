// SPDX-License-Identifier: GPL-3.0-or-later
#include "kinematics.hpp"
#include <algorithm>
#include <cmath>
#include <limits>

namespace physik {
static_assert(sizeof(double)==8&&std::numeric_limits<double>::digits>=53,"Kinematics requires binary64");
namespace {
enum class Mask { Unknown,State,TimeX,TimeV,AccelV,AccelX,InitialV,AccelDx };

Mask maskFor(const std::string& id){
  if(id.compare(0,4,"KIN_")!=0)return Mask::Unknown;
  if(id=="KIN_STATE")return Mask::State;
  if(id=="KIN_T_X")return Mask::TimeX;
  if(id=="KIN_T_V")return Mask::TimeV;
  if(id=="KIN_A_V")return Mask::AccelV;
  if(id=="KIN_A_X")return Mask::AccelX;
  if(id=="KIN_V0_X")return Mask::InitialV;
  if(id=="KIN_A_DX")return Mask::AccelDx;
  return Mask::Unknown;
}

std::size_t inputCount(Mask mask){
  switch(mask){
    case Mask::State:case Mask::TimeX:case Mask::AccelX:case Mask::InitialV:return 4;
    case Mask::TimeV:case Mask::AccelV:case Mask::AccelDx:return 3;
    case Mask::Unknown:return 0;
  }
  return 0;
}

// Keep exponents separate so intermediate products need not fit in binary64.
// Only final, nonzero quantities must survive conversion back to double.
struct Scaled {
  double fraction=0;
  int exponent=0;
  explicit Scaled(double value=0){fraction=std::frexp(value,&exponent);}
};

Scaled normalized(double fraction,int exponent){
  Scaled result(fraction);
  result.exponent+=exponent;
  return result;
}
Scaled negated(Scaled value){value.fraction=-value.fraction;return value;}
Scaled absolute(Scaled value){value.fraction=std::abs(value.fraction);return value;}
Scaled twice(Scaled value){++value.exponent;return value;}
Scaled half(Scaled value){--value.exponent;return value;}

double aligned(double fraction,int shift){
  // A term this far below the other addend cannot affect its rounded value.
  return shift< -1074?0:std::scalbn(fraction,shift);
}
Scaled added(Scaled a,Scaled b){
  if(a.fraction==0)return b;
  if(b.fraction==0)return a;
  const int exponent=std::max(a.exponent,b.exponent);
  return normalized(aligned(a.fraction,a.exponent-exponent)+
                    aligned(b.fraction,b.exponent-exponent),exponent);
}
Scaled subtracted(Scaled a,Scaled b){return added(a,negated(b));}
Scaled multiplied(Scaled a,Scaled b){
  return normalized(a.fraction*b.fraction,a.exponent+b.exponent);
}
Scaled divided(Scaled a,Scaled b){
  // Callers check all physical zero denominators before reaching here.
  return normalized(a.fraction/b.fraction,a.exponent-b.exponent);
}

Scaled sumProducts(Scaled a,Scaled b,Scaled c,Scaled d){
  if(a.fraction==0||b.fraction==0)return multiplied(c,d);
  if(c.fraction==0||d.fraction==0)return multiplied(a,b);
  const int pExponent=a.exponent+b.exponent,qExponent=c.exponent+d.exponent;
  const int exponent=std::max(pExponent,qExponent);
  const double p=a.fraction*b.fraction,q=c.fraction*d.fraction;
  const double pError=std::fma(a.fraction,b.fraction,-p);
  const double qError=std::fma(c.fraction,d.fraction,-q);
  const double x=aligned(p,pExponent-exponent),y=aligned(q,qExponent-exponent);
  const double sum=x+y,z=sum-x;
  const double correction=(x-(sum-z))+(y-z)+aligned(pError,pExponent-exponent)+
                          aligned(qError,qExponent-exponent);
  return normalized(sum+correction,exponent);
}

Scaled sumWithResidual(Scaled a,Scaled b,Scaled& residual){
  residual=Scaled(0);
  if(a.fraction==0)return b;
  if(b.fraction==0)return a;
  // Avoid underflow while retaining even a very remote low-order term.
  if(a.exponent-b.exponent>54){residual=b;return a;}
  if(b.exponent-a.exponent>54){residual=a;return b;}
  const int exponent=std::max(a.exponent,b.exponent);
  const double x=aligned(a.fraction,a.exponent-exponent);
  const double y=aligned(b.fraction,b.exponent-exponent);
  const double sum=x+y,z=sum-x;
  residual=normalized((x-(sum-z))+(y-z),exponent);
  return normalized(sum,exponent);
}

Scaled productWithResidual(Scaled a,Scaled b,Scaled& residual){
  const int exponent=a.exponent+b.exponent;
  const double product=a.fraction*b.fraction;
  residual=normalized(std::fma(a.fraction,b.fraction,-product),exponent);
  return normalized(product,exponent);
}

// These position polynomials have at most seven exact product/sum terms.
// A fixed expansion preserves cancellation and discriminant sign; it does not
// require arbitrary precision, allocation, or a tolerance on decimal inputs.
struct PositionSum {
  Scaled parts[8];
  std::size_t count=0;

  void add(Scaled value){
    std::size_t next=0;
    for(std::size_t i=0;i<count;++i){
      Scaled residual;
      value=sumWithResidual(value,parts[i],residual);
      if(residual.fraction!=0)parts[next++]=residual;
    }
    if(value.fraction!=0)parts[next++]=value;
    count=next;
  }
  void addProduct(Scaled a,Scaled b){
    Scaled residual;
    const Scaled product=productWithResidual(a,b,residual);
    add(residual);add(product);
  }
  Scaled value() const{
    Scaled result;
    for(std::size_t i=0;i<count;++i)result=added(result,parts[i]);
    return result;
  }
};

Scaled squareRoot(Scaled value){
  if(value.exponent%2!=0){value.fraction*=2;--value.exponent;}
  return normalized(std::sqrt(value.fraction),value.exponent/2);
}

bool fail(std::string& error,const char* message){error=message;return false;}
bool asDouble(Scaled value,double& result,std::string& error){
  result=std::scalbn(value.fraction,value.exponent);
  if(!std::isfinite(result))return fail(error,"Ergebnis zu gross fuer den Zahlenbereich");
  if(value.fraction!=0&&result==0)return fail(error,"Ergebnis zu klein fuer den Zahlenbereich (Unterlauf)");
  if(result==0)result=0; // Do not display negative zero.
  return true;
}
bool append(KinematicResult& result,const char* name,Scaled value,const char* unit,std::string& error){
  double number=0;
  if(!asDouble(value,number,error))return false;
  result.values.push_back({name,number,unit});
  return true;
}

constexpr const char* modelNote=
  "Modell: konstantes a; positive Achse frei gewaehlt. Zeiten seit Phasenbeginn (t >= 0). "
  "Negative Geschwindigkeiten und Beschleunigungen sind zulaessig.";

void rootNote(std::string& note,const char* label,double v0,double a,double v){
  note+="\n";note+=label;note+=": ";
  if(a!=0&&v0!=0&&((a<0)!=(v0<0))){
    note+=v==0?"am Umkehrpunkt":((v<0)==(v0<0)?"vor der Umkehr":"nach der Umkehr");
  }else if(a!=0&&v==0){
    note+="Umkehrpunkt am Phasenbeginn";
  }else{
    note+="ohne Umkehr seit Phasenbeginn";
  }
  note+=v==0?", Stillstand.":(v<0?", Bewegung in -Richtung.":", Bewegung in +Richtung.");
}

bool timeFromPosition(const std::vector<double>& inputs,KinematicResult& result,std::string& error){
  const double startV=inputs[1],acceleration=inputs[2];
  const Scaled v0(startV),a(acceleration),dx=subtracted(Scaled(inputs[3]),Scaled(inputs[0]));
  if(acceleration==0){
    if(startV==0){
      return fail(error,dx.fraction==0?
        "Nicht eindeutig: Stillstand am Ziel, jede Zeit t >= 0 passt":
        "Keine physikalische Loesung: Stillstand ausserhalb des Ziels");
    }
    const Scaled time=divided(dx,v0);
    if(time.fraction<0)return fail(error,"Keine physikalische Loesung fuer t >= 0");
    if(!append(result,"t",time,"s",error)||!append(result,"v",v0,"m/s",error))return false;
    rootNote(result.note,"t",startV,acceleration,startV);
    return true;
  }

  // Solve a*t^2/2 + v0*t - dx = 0 using q, avoiding root cancellation.
  PositionSum discriminantTerms;
  discriminantTerms.addProduct(v0,v0);
  discriminantTerms.addProduct(twice(a),Scaled(inputs[3]));
  discriminantTerms.addProduct(negated(twice(a)),Scaled(inputs[0]));
  const Scaled discriminant=discriminantTerms.value();
  if(discriminant.fraction<0)return fail(error,"Keine physikalische Loesung: Ziel wird nicht erreicht");
  struct Root {Scaled time,velocity;double seconds=0,speed=0;};
  Root roots[2];
  std::size_t count=0;
  if(discriminant.fraction==0){
    const Scaled time=divided(negated(v0),a);
    if(time.fraction>=0)roots[count++]={time,Scaled(0),0,0};
  }else{
    const Scaled speed=squareRoot(discriminant);
    const Scaled signedSpeed=startV<0?negated(speed):speed;
    const Scaled q=negated(half(added(v0,signedSpeed)));
    const Scaled first=divided(twice(q),a),second=divided(negated(dx),q);
    if(first.fraction>=0)roots[count++]={first,negated(signedSpeed),0,0};
    if(second.fraction>=0)roots[count++]={second,signedSpeed,0,0};
  }
  if(count==0)return fail(error,"Keine physikalische Loesung fuer t >= 0");
  for(std::size_t i=0;i<count;++i){
    if(!asDouble(roots[i].time,roots[i].seconds,error)||
       !asDouble(roots[i].velocity,roots[i].speed,error))return false;
  }
  if(count==2){
    if(roots[1].seconds<roots[0].seconds)std::swap(roots[0],roots[1]);
    if(roots[0].seconds==roots[1].seconds){
      // Distinct roots must not silently become one double root by rounding.
      return fail(error,"Zwei Zeiten im Zahlenbereich nicht unterscheidbar");
    }
  }
  for(std::size_t i=0;i<count;++i){
    const char* time=count==1?"t":(i==0?"t_1":"t_2");
    const char* velocity=count==1?"v":(i==0?"v_1":"v_2");
    result.values.push_back({time,roots[i].seconds,"s"});
    result.values.push_back({velocity,roots[i].speed,"m/s"});
    rootNote(result.note,time,startV,acceleration,roots[i].speed);
  }
  return true;
}
}

const std::vector<Calculation>& kinematicsCatalog(){
  static const std::vector<Calculation> catalog{
    {"Ort, Geschwindigkeit und Weg","x=x0+v0*t+a*t^2/2; v=v0+a*t","x","m","KIN_STATE",
      {{"Startort x0","m","m"},{"Startgeschw. v0","m/s","m/s"},
       {"Beschleunigung a","m/s^2","m/s^2"},{"Zeit t","s","s"}}},
    {"Zeit bis zum Zielort","x=x0+v0*t+a*t^2/2","t","s","KIN_T_X",
      {{"Startort x0","m","m"},{"Startgeschw. v0","m/s","m/s"},
       {"Beschleunigung a","m/s^2","m/s^2"},{"Zielort x","m","m"}}},
    {"Zeit bis zur Geschwindigkeit","t=(v-v0)/a","t","s","KIN_T_V",
      {{"Startgeschw. v0","m/s","m/s"},{"Beschleunigung a","m/s^2","m/s^2"},
       {"Endgeschw. v","m/s","m/s"}}},
    {"Beschleunigung aus Geschwindigkeiten","a=(v-v0)/t","a","m/s^2","KIN_A_V",
      {{"Startgeschw. v0","m/s","m/s"},{"Endgeschw. v","m/s","m/s"},{"Zeit t","s","s"}}},
    {"Beschleunigung aus Zielort","a=2*(x-x0-v0*t)/t^2","a","m/s^2","KIN_A_X",
      {{"Startort x0","m","m"},{"Startgeschw. v0","m/s","m/s"},
       {"Zeit t","s","s"},{"Zielort x","m","m"}}},
    {"Startgeschwindigkeit aus Zielort","v0=(x-x0)/t-a*t/2","v0","m/s","KIN_V0_X",
      {{"Startort x0","m","m"},{"Beschleunigung a","m/s^2","m/s^2"},
       {"Zeit t","s","s"},{"Zielort x","m","m"}}},
    {"Beschleunigung aus Verschiebung","t=2*Delta_x/(v0+v); a=(v-v0)/t","a","m/s^2","KIN_A_DX",
      {{"Startgeschw. v0","m/s","m/s"},{"Endgeschw. v","m/s","m/s"},
       {"Verschiebung Delta_x","m","m"}}}
  };
  return catalog;
}

bool isKinematic(const std::string& id){return maskFor(id)!=Mask::Unknown;}

std::vector<Unit> kinematicsUnits(const Parameter& parameter){
  if(parameter.unit=="m")return {{"m",1},{"km",1000},{"cm",.01},{"mm",.001}};
  if(parameter.unit=="s")return {{"s",1},{"min",60},{"h",3600},{"ms",.001}};
  if(parameter.unit=="m/s")return {{"m/s",1},{"km/h",1/3.6}};
  return unitOptions(parameter.unit);
}

bool calculateKinematics(const Calculation& calc,const std::vector<double>& inputs,
                         KinematicResult& result,std::string& error){
  result={};error.clear();
  const Mask mask=maskFor(calc.id);
  if(mask==Mask::Unknown)return fail(error,"Unbekannte Kinematikformel");
  if(inputs.size()!=inputCount(mask)||calc.parameters.size()!=inputCount(mask))
    return fail(error,"Falsche Anzahl Eingaben oder unpassende Maskendaten");
  for(double input:inputs)if(!std::isfinite(input))return fail(error,"Eingabe nicht endlich / zu gross");
  KinematicResult calculated;calculated.note=modelNote;
  bool success=false;
  switch(mask){
    case Mask::State:{
      if(inputs[3]<0)return fail(error,"Zeit t muss >= 0 sein");
      const Scaled x0(inputs[0]),v0(inputs[1]),a(inputs[2]),t(inputs[3]);
      Scaled atResidual;
      const Scaled at=productWithResidual(a,t,atResidual);
      PositionSum velocityTerms;
      velocityTerms.add(v0);velocityTerms.add(atResidual);velocityTerms.add(at);
      const Scaled v=velocityTerms.value();
      PositionSum positionTerms;
      positionTerms.addProduct(v0,t);
      positionTerms.addProduct(at,half(t));
      positionTerms.addProduct(atResidual,half(t));
      const Scaled dx=positionTerms.value();
      positionTerms.add(x0);
      const Scaled x=positionTerms.value();
      const bool reversal=v0.fraction!=0&&v.fraction!=0&&((v0.fraction<0)!=(v.fraction<0));
      const Scaled path=reversal?
        divided(sumProducts(v0,v0,v,v),twice(absolute(a))):absolute(dx);
      success=append(calculated,"x",x,"m",error)&&append(calculated,"Delta_x",dx,"m",error)&&
              append(calculated,"v",v,"m/s",error)&&append(calculated,"Weg",path,"m",error);
      calculated.note+=reversal?"\nWeg enthaelt die Umkehr; Delta_x ist die signierte Verschiebung.":
                                "\nOhne innere Umkehr: Weg = abs(Delta_x).";
      break;
    }
    case Mask::TimeX:
      success=timeFromPosition(inputs,calculated,error);
      break;
    case Mask::TimeV:{
      const Scaled v0(inputs[0]),a(inputs[1]),v(inputs[2]);
      if(inputs[1]==0)return fail(error,inputs[2]==inputs[0]?
        "Nicht eindeutig: a = 0 und v = v0, jede Zeit t >= 0 passt":
        "Keine physikalische Loesung: a = 0 und v != v0");
      const Scaled t=divided(subtracted(v,v0),a);
      if(t.fraction<0)return fail(error,"Keine physikalische Loesung fuer t >= 0");
      const Scaled dx=multiplied(half(added(v0,v)),t);
      success=append(calculated,"t",t,"s",error)&&append(calculated,"Delta_x",dx,"m",error);
      break;
    }
    case Mask::AccelV:{
      if(inputs[2]<=0)return fail(error,"Zur Bestimmung von a muss Zeit t > 0 sein");
      const Scaled v0(inputs[0]),v(inputs[1]),t(inputs[2]);
      const Scaled a=divided(subtracted(v,v0),t),dx=multiplied(half(added(v0,v)),t);
      success=append(calculated,"a",a,"m/s^2",error)&&append(calculated,"Delta_x",dx,"m",error);
      break;
    }
    case Mask::AccelX:{
      if(inputs[2]<=0)return fail(error,"Zur Bestimmung von a muss Zeit t > 0 sein");
      const Scaled x0(inputs[0]),v0(inputs[1]),t(inputs[2]),x(inputs[3]);
      PositionSum residual;
      residual.add(x);residual.add(negated(x0));residual.addProduct(negated(v0),t);
      const Scaled a=divided(twice(residual.value()),multiplied(t,t));
      const Scaled v=sumProducts(a,t,v0,Scaled(1));
      success=append(calculated,"a",a,"m/s^2",error)&&append(calculated,"v",v,"m/s",error);
      break;
    }
    case Mask::InitialV:{
      if(inputs[2]<=0)return fail(error,"Zur Bestimmung von v0 muss Zeit t > 0 sein");
      const Scaled x0(inputs[0]),a(inputs[1]),t(inputs[2]),x(inputs[3]);
      Scaled changeResidual;
      const Scaled halfChange=productWithResidual(a,half(t),changeResidual);
      PositionSum initialVelocity,finalVelocity;
      initialVelocity.add(x);initialVelocity.add(negated(x0));
      initialVelocity.addProduct(negated(halfChange),t);
      initialVelocity.addProduct(negated(changeResidual),t);
      finalVelocity.add(x);finalVelocity.add(negated(x0));
      finalVelocity.addProduct(halfChange,t);finalVelocity.addProduct(changeResidual,t);
      const Scaled v0=divided(initialVelocity.value(),t),v=divided(finalVelocity.value(),t);
      success=append(calculated,"v0",v0,"m/s",error)&&append(calculated,"v",v,"m/s",error);
      break;
    }
    case Mask::AccelDx:{
      const Scaled v0(inputs[0]),v(inputs[1]),dx(inputs[2]),sum=added(v0,v);
      if(dx.fraction==0)return fail(error,sum.fraction==0?
        "Nicht eindeutig: Delta_x = 0 und v0 + v = 0, a und t sind nicht festgelegt":
        "Keine physikalische Loesung mit t > 0 bei Delta_x = 0");
      if(sum.fraction==0)return fail(error,"Keine physikalische Loesung: mittlere Geschwindigkeit ist 0 bei Delta_x != 0");
      const Scaled t=divided(twice(dx),sum);
      if(t.fraction<=0)return fail(error,"Keine physikalische Loesung mit t > 0 fuer diese Vorzeichen");
      const Scaled a=divided(subtracted(v,v0),t);
      success=append(calculated,"a",a,"m/s^2",error)&&append(calculated,"t",t,"s",error);
      calculated.note+="\nDelta_x ist signiert, nicht der zurueckgelegte Weg; t > 0.";
      break;
    }
    case Mask::Unknown:break;
  }
  if(!success)return false;
  result=std::move(calculated);
  return true;
}
}

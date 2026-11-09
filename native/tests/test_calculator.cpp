// SPDX-License-Identifier: GPL-3.0-or-later
#include "calculator.hpp"
#include <algorithm>
#include <bit>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <cstdlib>
#include <limits>

namespace {
int checks=0;
void need(bool value,const std::string& message){++checks;if(!value){std::cerr<<"FAIL CALC: "<<message<<'\n';std::exit(1);}}
uint32_t u32(std::ifstream& input){uint8_t b[4]{};input.read(reinterpret_cast<char*>(b),4);need(input.good(),"fixture integer");return uint32_t(b[0])<<24|uint32_t(b[1])<<16|uint32_t(b[2])<<8|b[3];}
double real(std::ifstream& input){uint8_t b[8]{};input.read(reinterpret_cast<char*>(b),8);need(input.good(),"fixture double");uint64_t bits=0;for(uint8_t byte:b)bits=(bits<<8)|byte;return std::bit_cast<double>(bits);}
bool close(double a,double b,double absolute=0){return std::isfinite(a)&&std::abs(a-b)<=std::max(absolute,std::abs(b)*2e-9);}
}
int main(){
  using namespace physik;
  Store store("../");std::map<std::string,Calculation> calcs;std::string error;
  for(uint32_t i=0;i<calculations.count;++i){Value value;Calculation calc;need(store.row(calculations,i,value),store.error());need(calculationRecord(value,calc,error),error);calcs.emplace(calc.id,calc);}
  need(calcs.size()==167,"all calculators loaded");
  std::ifstream input("build/reference/calculations.ref",std::ios::binary);const uint32_t cases=u32(input);std::set<std::string> covered;
  for(uint32_t i=0;i<cases;++i){
    const uint32_t length=u32(input);need(length<100,"bounded id");std::string id(length,'\0');input.read(id.data(),length);
    const uint32_t count=u32(input);need(count<=4,"bounded inputs");std::vector<double> values;for(uint32_t j=0;j<count;++j)values.push_back(real(input));
    char ok=0;input.read(&ok,1);const double expected=real(input);double actual=0;
    const bool success=calculate(calcs.at(id),values,actual,error);
    need(success==(ok!=0),id+" domain: "+error);
    if(ok)need(close(actual,expected),id+" expected "+formatNumber(expected)+", got "+formatNumber(actual));
    covered.insert(id);
  }
  need(covered.size()==163,"all fixed equations covered");
  for(const auto& [id,calc]:calcs){
    if(id.rfind("FIT_",0)==0)continue;
    double value=0;need(!calculate(calc,{},value,error),"empty parameters rejected");
    Calculation bad=calc;bad.parameters.push_back({"extra","1","1"});
    need(!calculate(bad,std::vector<double>(bad.parameters.size(),2),value,error),"corrupt metadata arity rejected");
    auto values=std::vector<double>(calc.parameters.size(),1);values[0]=std::numeric_limits<double>::infinity();
    need(!calculate(calc,values,value,error),"infinite input rejected");
  }
  double value=0;
  for(const auto& text:{"6.626e-34","-6.626e-34","1e-100","1E30","1,5","0E-999"})need(parseNumber(text,1,value,error),std::string("parse ")+text);
  for(const auto& text:{"nan","inf","1e999","1E-999","1e","1.2.3","","1/0"})need(!parseNumber(text,1,value,error),std::string("reject ")+text);
  for(const auto& base:{"m","J","V","s","Hz","kg"})for(const auto& unit:unitOptions(base)){need(parseNumber("2",unit.scale,value,error),"unit parse");need(close(value,2*unit.scale),"unit scale");}
  need(formatNumber(6.626e-34)!="0"&&formatNumber(-6.626e-34)!="0","small results not rounded to zero");
  std::vector<double> displayCases={0.,-0.,.25,-2.479683968664005,6.62607015e-34,1e-100,1e-4,1e-5,999999.9,
    std::numeric_limits<double>::denorm_min(),std::numeric_limits<double>::min(),std::numeric_limits<double>::max()};
  for(int exponent=-320;exponent<=308;exponent+=7){const double value=1.23456789*std::pow(10.,exponent);displayCases.push_back(value);displayCases.push_back(-value);}
  for(double display:displayCases){char expected[48]{};std::snprintf(expected,sizeof(expected),"%.6g",display);
    need(formatNumber(display)==(display==0?"0":expected),"native scientific formatter "+std::string(expected));}
  need(calculate(calcs.at("T_FROM_F"),{2},value,error)&&value==.5,"period reference");
  need(calculate(calcs.at("T_SPRING"),{1,4*pi*pi},value,error)&&close(value,1),"spring reference");
  need(calculate(calcs.at("Q_EL"),{500e-9},value,error)&&close(value/charge,2.479683968664005),"photon reference");
  need(calculate(calcs.at("Q_BOX_E"),{1e-9,1},value,error)&&close(value/charge,.37603016210487256),"box reference");
  std::ifstream fits("build/reference/fits.ref",std::ios::binary);const uint32_t fitCases=u32(fits);
  for(uint32_t i=0;i<fitCases;++i){const bool wavelength=u32(fits)!=0;const uint32_t count=u32(fits);std::vector<std::pair<double,double>> points;
    for(uint32_t j=0;j<count;++j){const double x=real(fits),y=real(fits);points.emplace_back(x,y);}
    const double h=real(fits),work=real(fits),threshold=real(fits),rms=real(fits);Fit result;
    need(fitPlanck(points,wavelength,result,error),error);need(close(result.h,h)&&close(result.work,work)&&close(result.threshold,threshold)&&close(result.rms,rms,1e-12),"Planck fit reference");
  }
  for(const auto& points:std::vector<std::vector<std::pair<double,double>>>{{{1,1}},{{1,1},{1,2}},{{1,2},{2,1}},{{1,2},{2,3}}}){Fit result;need(!fitPlanck(points,false,result,error),"invalid fit rejected");}
  std::cout<<"PASS CALC: "<<checks<<" checks; 163 fixed formulas; 4 fit workflows; "<<cases<<" formula cases; "<<fitCases<<" fits\n";
}

// SPDX-License-Identifier: GPL-3.0-or-later
#include "catalog_data.hpp"
#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <map>

using namespace physik;
namespace {
unsigned checks=0;
void require(bool ok,const std::string& why){++checks;if(!ok){std::cerr<<"FAIL CATALOG: "<<why<<'\n';std::exit(1);}}
uint32_t u32(std::ifstream& in){uint8_t b[4]{};in.read(reinterpret_cast<char*>(b),4);require(in.good(),"fixture length");return uint32_t(b[0])<<24|uint32_t(b[1])<<16|uint32_t(b[2])<<8|b[3];}
std::string next(std::ifstream& in){const auto n=u32(in);require(n>0&&n<4096,"fixture bound");std::string s(n,'\0');in.read(s.data(),s.size());require(in.good(),"fixture read");return s;}
Value text(const std::string& s){Value v;v.type=Value::Text;v.text=s;return v;}
Value integer(unsigned n){Value v;v.type=Value::Integer;v.number=n;return v;}
Value tuple(std::vector<Value> items){Value v;v.type=Value::Tuple;v.items=std::move(items);return v;}
Value strings(const std::vector<std::string>& items){std::vector<Value> result;for(const auto& s:items)result.push_back(text(s));return tuple(std::move(result));}
Value params(const std::vector<Parameter>& items){std::vector<Value> result;for(const auto& p:items)result.push_back(tuple({text(p.name),text(p.unit),text(p.defaultUnit)}));return tuple(std::move(result));}
std::string encode(const Workflow& w){const auto& c=w.calculation;return encodeRecord(tuple({text(c.title),text(c.formula),text(c.name),text(c.unit),text(c.id),params(c.parameters),strings(w.defaults),integer(w.minCount),integer(w.maxCount),params(w.repeated),strings(w.repeatedDefaults)}));}
void write(const std::string& data){std::ofstream out("build/catalog_bad/wissen.txt",std::ios::binary);out.write(data.data(),data.size());require(out.good(),"bad fixture write");}
}
int main(){
  std::ifstream fixture("build/catalog-records.bin",std::ios::binary);std::string marker(10,'\0');fixture.read(marker.data(),10);require(marker=="PCNATIVE1\n","export header");
  std::map<std::string,std::vector<std::string>> expected;
  while(fixture.peek()!=std::char_traits<char>::eof()){const auto category=next(fixture);expected[category].push_back(next(fixture));}
  nativeDataDirectory("build/package/");size_t count=0;
  for(const auto& [category,records]:expected){
    const auto loaded=dataCatalog(category);require(loaded.size()==records.size(),category+": "+nativeCatalogError());
    for(size_t i=0;i<records.size();++i){
      require(encode(loaded[i])==records[i],"all metadata byte equal: "+loaded[i].calculation.id);
      Workflow one;require(dataWorkflow(loaded[i].calculation.id,one)&&encode(one)==records[i],"single workflow lookup");++count;
    }
  }
  require(count==254&&expected.size()==32,"all exported workflows");
  require(dataCatalog("missing").empty(),"unknown category");
  Workflow absent;require(!dataWorkflow("missing",absent),"unknown workflow");
  std::ifstream file("build/package/wissen.txt",std::ios::binary);require(file.good(),"knowledge file");const std::string bytes{std::istreambuf_iterator<char>(file),{}};
  const auto start=bytes.find("PCCATALOG3 ");require(start!=std::string::npos,"catalog marker present");
  std::filesystem::create_directories("build/catalog_bad");auto bad=bytes;bad[start+12]^=1;write(bad);nativeDataDirectory("build/catalog_bad/");
  const auto first=nativeCategories().front().id;
  require(dataCatalog(first).empty()&&nativeCatalogError().find("drei")!=std::string::npos,"mismatched package explained");
  Store oldPart("build/catalog_bad/");std::string article;require(oldPart.chapter(0,article),"base knowledge survives catalog mismatch");
  bad=bytes;const auto textAt=bad.find(expected[first][0],start);require(textAt!=std::string::npos,"record position");bad[textAt+10]^=1;write(bad);
  require(dataCatalog(first).empty()&&nativeCatalogError().find("beschaedigt")!=std::string::npos,"category checksum rejects valid-looking changed text");
  write(bytes.substr(0,textAt+20));require(dataCatalog(first).empty()&&!nativeCatalogError().empty(),"truncated category");
  write(bytes.substr(0,start));require(dataCatalog(first).empty(),"missing catalog block");
  nativeDataDirectory("build/package/");require(dataCatalog(first).size()==expected[first].size(),"recovery after wrong data");
  std::cout<<"PASS CATALOG: "<<checks<<" checks; "<<count<<" exact workflows; "<<expected.size()<<" categories\n";
}

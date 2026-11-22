// SPDX-License-Identifier: GPL-3.0-or-later
#include "extensions.hpp"
#include <fstream>
#include <iostream>

using namespace physik;
namespace {
Value text(const std::string& s){Value v;v.type=Value::Text;v.text=s;return v;}
Value number(unsigned n){Value v;v.type=Value::Integer;v.number=n;return v;}
Value tuple(std::vector<Value> values){Value v;v.type=Value::Tuple;v.items=std::move(values);return v;}
Value strings(const std::vector<std::string>& values){std::vector<Value> result;for(const auto& s:values)result.push_back(text(s));return tuple(std::move(result));}
Value params(const std::vector<Parameter>& values){std::vector<Value> result;for(const auto& p:values)result.push_back(tuple({text(p.name),text(p.unit),text(p.defaultUnit)}));return tuple(std::move(result));}
void sized(std::ofstream& out,const std::string& bytes){const uint32_t length=bytes.size();for(int shift=24;shift>=0;shift-=8)out.put((length>>shift)&255);out.write(bytes.data(),bytes.size());}
}
int main(){
  std::ofstream out("build/catalog-records.bin",std::ios::binary);out<<"PCNATIVE1\n";unsigned count=0;
  for(const auto& category:nativeCategories())for(const auto& w:nativeCatalog(category.id)){
    const auto& c=w.calculation;
    const auto record=encodeRecord(tuple({text(c.title),text(c.formula),text(c.name),text(c.unit),text(c.id),params(c.parameters),
      strings(w.defaults),number(w.minCount),number(w.maxCount),params(w.repeated),strings(w.repeatedDefaults)}));
    sized(out,category.id);sized(out,record);++count;
  }
  out.close();if(!out.good())return 1;
  std::cout<<"PASS EXPORT: "<<count<<" native workflow records\n";
}

// SPDX-License-Identifier: GPL-3.0-or-later
#include "knowledge.hpp"
#include <algorithm>
#include <cstring>

namespace physik {
namespace {
struct SupplementEntry {const char* title;const char* group;const char* quantities;const char* media;uint32_t start,length,checksum;};
#include "knowledge_index.inc"
std::vector<std::string> split(const std::string& text){
  std::vector<std::string> result;size_t begin=0;
  while(begin<text.size()){const auto end=text.find('|',begin);result.push_back(text.substr(begin,end==std::string::npos?end:end-begin));if(end==std::string::npos)break;begin=end+1;}
  return result;
}
}
size_t supplementCount(){return sizeof(supplementIndex)/sizeof(supplementIndex[0]);}
size_t supplementLinkCount(){size_t count=2*supplementCount();for(const auto& entry:supplementIndex)count+=split(entry.quantities).size();return count;}
std::vector<NativeCategory> supplementSections(int mode){
  std::vector<NativeCategory> result;
  for(const auto& entry:supplementIndex){const auto names=mode==1?split(entry.quantities):std::vector<std::string>{entry.group};
    for(const auto& name:names){const std::string id="SUP:"+std::to_string(mode)+":"+name;
      if(std::none_of(result.begin(),result.end(),[&](const NativeCategory& c){return c.id==id;}))result.push_back({id,"Neu: "+name});
    }
  }
  return result;
}
std::vector<std::pair<std::string,uint32_t>> supplementEntries(const std::string& category){
  std::vector<std::pair<std::string,uint32_t>> result;
  if(category.size()<7||category.rfind("SUP:",0)!=0||category[5]!=':'||category[4]<'0'||category[4]>'2')return result;
  const auto name=category.substr(6);const bool quantity=category[4]=='1';
  for(uint32_t i=0;i<supplementCount();++i){const auto& entry=supplementIndex[i];const auto names=quantity?split(entry.quantities):std::vector<std::string>{entry.group};
    if(std::find(names.begin(),names.end(),name)!=names.end())result.emplace_back(entry.title,i);
  }
  return result;
}
bool supplementLoad(Store& store,uint32_t index,std::string& text,std::vector<std::string>& media,std::string& error){
  text.clear();media.clear();error.clear();
  if(index>=supplementCount()){error="Unbekannter Ergaenzungsartikel";return false;}
  std::vector<uint8_t> data;
  if(!store.read(0,supplementOffset,std::strlen(supplementMarker),data)||std::string(data.begin(),data.end())!=supplementMarker){
    error="Wissenserweiterung passt nicht zur App. Alle drei Paketdateien zusammen austauschen.\n"+store.error();return false;
  }
  const auto& entry=supplementIndex[index];
  if(!store.read(0,entry.start,entry.length,data)){error=store.error();return false;}
  uint32_t hash=2166136261u;for(uint8_t byte:data)hash=(hash^byte)*16777619u;
  if(hash!=entry.checksum||!std::all_of(data.begin(),data.end(),[](uint8_t b){return b>0&&b<128;})){
    error="Wissensartikel beschaedigt; vollstaendiges Paket erneut uebertragen.";return false;
  }
  text.assign(data.begin(),data.end());media=split(entry.media);return true;
}
}

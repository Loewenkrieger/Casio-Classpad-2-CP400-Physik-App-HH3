// SPDX-License-Identifier: GPL-3.0-or-later
#include "navigation.hpp"
#include "knowledge.hpp"
#include <algorithm>
#include <cstring>

namespace physik {
namespace {
#include "navigation_index.inc"
uint32_t checksum(const std::vector<uint8_t>& bytes){
  uint32_t hash=2166136261u;for(uint8_t b:bytes)hash=(hash^b)*16777619u;return hash;
}
bool integer(const Value& value,uint32_t& output){
  if(value.type!=Value::Integer||value.number<0||value.number>UINT32_MAX)return false;
  output=static_cast<uint32_t>(value.number);return true;
}
bool fail(std::string& error,const char* message){error=message;return false;}
}
uint32_t navigationNodeCount(){return navigationDirectory.count;}
uint32_t navigationRoot(unsigned mode){return mode<3?mode:mode==3?navigationAlphabetRoot:UINT32_MAX;}
bool navigationRows(Store& store,uint32_t node,std::vector<NavigationItem>& output,std::string& error){
  output.clear();error.clear();
  if(node>=navigationDirectory.count)return fail(error,"Unbekannter Navigationsknoten");
  std::vector<uint8_t> bytes;
  if(!store.read(0,navigationOffset,std::strlen(navigationMarker),bytes)||std::string(bytes.begin(),bytes.end())!=navigationMarker){
    error="Navigation passt nicht zur App. Alle drei Paketdateien gemeinsam ersetzen.\n"+store.error();return false;
  }
  Value record;
  if(!store.row(navigationDirectory,node,record)){error="Navigationsverzeichnis nicht lesbar.\n"+store.error();return false;}
  if(record.type!=Value::Tuple||record.items.size()!=5)return fail(error,"Ungueltiges Navigationsverzeichnis");
  uint32_t fields[5]{};
  for(unsigned i=0;i<5;++i)if(!integer(record.items[i],fields[i]))return fail(error,"Ungueltige Navigationsposition");
  uint32_t hash=2166136261u;
  for(unsigned i=0;i<4;++i)for(int shift=24;shift>=0;shift-=8)hash=(hash^((fields[i]>>shift)&255))*16777619u;
  if(hash!=fields[4])return fail(error,"Navigationsverzeichnis beschaedigt");
  const auto start=fields[0],count=fields[1],width=fields[2];
  const uint64_t endDirectory=static_cast<uint64_t>(navigationDirectory.start)+static_cast<uint64_t>(navigationDirectory.count)*navigationDirectory.width;
  const uint64_t length=static_cast<uint64_t>(count)*width;
  if(count==0||count>256||width==0||width>512||length>131072||start<endDirectory)
    return fail(error,"Ungueltiger Navigationsbereich");
  if(!store.read(0,start,static_cast<uint32_t>(length),bytes)){error="Menue nicht lesbar.\n"+store.error();return false;}
  if(checksum(bytes)!=fields[3])return fail(error,"Navigationsmenue beschaedigt; Paket erneut uebertragen.");
  std::vector<NavigationItem> rows;rows.reserve(count);
  for(uint32_t i=0;i<count;++i){
    const auto begin=bytes.begin()+i*width;Value value;
    if(!decodeRecord(std::string(begin,begin+width),value,error))return false;
    if(value.type!=Value::Tuple||value.items.size()!=3||value.items[0].type!=Value::Text)
      return fail(error,"Ungueltiger Navigationseintrag");
    const auto& label=value.items[0].text;uint32_t kind=0,target=0;
    if(label.empty()||label.size()>96||!std::all_of(label.begin(),label.end(),[](unsigned char c){return c>=32&&c<127;})||
       !integer(value.items[1],kind)||!integer(value.items[2],target)||kind>2)
      return fail(error,"Ungueltige Menuebeschriftung oder Referenz");
    if((kind==0&&(target<3||target==navigationAlphabetRoot||target<=node||target>=navigationDirectory.count))||
       (kind==1&&target>=chapters.count)||(kind==2&&target>=supplementCount()))
      return fail(error,"Navigationsziel liegt ausserhalb des Pakets");
    if(!rows.empty()&&((kind==0)!=(rows.front().kind==NavigationKind::Menu)))
      return fail(error,"Unvereinbare Navigationsknoten");
    for(const auto& previous:rows)if(previous.label==label||(static_cast<uint32_t>(previous.kind)==kind&&previous.target==target))
      return fail(error,"Doppelter Navigationseintrag");
    rows.push_back({label,static_cast<NavigationKind>(kind),target});
  }
  output=std::move(rows);return true;
}
}

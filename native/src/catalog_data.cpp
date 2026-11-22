// SPDX-License-Identifier: GPL-3.0-or-later
#include "catalog_data.hpp"
#include <algorithm>
#include <cstring>

namespace physik {
#ifdef PHYSIK_DATA_CATALOG
namespace {
struct CatalogEntry {const char* id;uint32_t start,count,width,checksum;};
#include "catalog_index.inc"
#ifdef PHYSIK_HH3
std::string directory="\\fls0\\";
#else
std::string directory="build/package/";
#endif
std::string lastCategory,catalogError;
bool texts(const Value& value,std::vector<std::string>& strings){
  if(value.type!=Value::Tuple)return false;
  for(const auto& item:value.items){if(item.type!=Value::Text)return false;strings.push_back(item.text);}return true;
}
bool parameters(const Value& value,std::vector<Parameter>& params){
  if(value.type!=Value::Tuple||value.items.size()>24)return false;
  for(const auto& item:value.items){std::vector<std::string> parts;if(!texts(item,parts)||parts.size()!=3)return false;
    params.push_back({parts[0],parts[1],parts[2]});}return true;
}
bool decodeWorkflow(const Value& value,Workflow& result){
  if(value.type!=Value::Tuple||value.items.size()!=11)return false;
  const auto& v=value.items;
  for(unsigned i=0;i<5;++i)if(v[i].type!=Value::Text)return false;
  if(v[7].type!=Value::Integer||v[8].type!=Value::Integer||v[7].number<0||v[8].number>40||v[7].number>v[8].number)return false;
  Workflow w;w.calculation={v[0].text,v[1].text,v[2].text,v[3].text,v[4].text,{}};
  w.minCount=v[7].number;w.maxCount=v[8].number;
  if(!parameters(v[5],w.calculation.parameters)||!texts(v[6],w.defaults)||!parameters(v[9],w.repeated)||!texts(v[10],w.repeatedDefaults))return false;
  if(w.calculation.id.empty()||w.calculation.parameters.empty()||w.defaults.size()>w.calculation.parameters.size()||w.repeatedDefaults.size()>w.repeated.size())return false;
  if(w.maxCount&&(!w.minCount||w.repeated.empty()))return false;
  result=std::move(w);return true;
}
}
void nativeDataDirectory(const std::string& value){directory=value;lastCategory.clear();catalogError.clear();}
const std::string& nativeCatalogError(){return catalogError;}
std::vector<Workflow> dataCatalog(const std::string& category){
  catalogError.clear();
  const CatalogEntry* entry=nullptr;for(const auto& candidate:catalogIndex)if(category==candidate.id){entry=&candidate;break;}
  if(!entry)return {};
  Store store(directory);std::vector<uint8_t> marker;
  if(!store.read(0,catalogOffset,std::strlen(catalogMarker),marker)||std::string(marker.begin(),marker.end())!=catalogMarker){
    catalogError="Rechnerkatalog passt nicht zur App. Alle drei Paketdateien gemeinsam ersetzen.\n"+store.error();return {};
  }
  if(entry->width==0||entry->width>4096||static_cast<uint64_t>(entry->count)*entry->width>131072){catalogError="Ungueltiger Katalogbereich";return {};}
  std::vector<uint8_t> bytes;
  if(!store.read(0,entry->start,entry->count*entry->width,bytes)){catalogError=store.error();return {};}
  uint32_t hash=2166136261u;for(uint8_t b:bytes)hash=(hash^b)*16777619u;
  if(hash!=entry->checksum){catalogError="Rechnerkatalog beschaedigt; vollstaendiges Paket erneut uebertragen.";return {};}
  std::vector<Workflow> result;result.reserve(entry->count);
  for(uint32_t i=0;i<entry->count;++i){Value record;const auto start=bytes.begin()+i*entry->width;
    if(!decodeRecord(std::string(start,start+entry->width),record,catalogError))return {};
    Workflow w;if(!decodeWorkflow(record,w)){catalogError="Beschaedigter Rechnerkatalog";return {};}
    result.push_back(std::move(w));}
  lastCategory=category;return result;
}
bool dataWorkflow(const std::string& id,Workflow& result){
  if(!lastCategory.empty())for(const auto& w:dataCatalog(lastCategory))if(w.calculation.id==id){result=w;return true;}
  for(const auto& category:nativeCategories())for(const auto& w:dataCatalog(category.id))if(w.calculation.id==id){result=w;return true;}
  result={};return false;
}
std::vector<Workflow> mechanicsCatalog(const std::string& category){return category.rfind("M_",0)==0?dataCatalog(category):std::vector<Workflow>{};}
std::vector<Workflow> fieldsCatalog(const std::string& category){return category.rfind("E_",0)==0?dataCatalog(category):std::vector<Workflow>{};}
std::vector<Workflow> opticsCatalog(const std::string& category){return category.rfind("O_",0)==0?dataCatalog(category):std::vector<Workflow>{};}
std::vector<Workflow> analysisCatalog(const std::string& category){return category.rfind("A_",0)==0?dataCatalog(category):std::vector<Workflow>{};}
std::vector<Workflow> simulationCatalog(const std::string& category){return category.rfind("S_",0)==0?dataCatalog(category):std::vector<Workflow>{};}
#else
void nativeDataDirectory(const std::string&){}
const std::string& nativeCatalogError(){static const std::string empty;return empty;}
#endif
}

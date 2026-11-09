// SPDX-License-Identifier: GPL-3.0-or-later
#include "store.hpp"
#include <algorithm>
#include <climits>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <limits>
#ifdef PHYSIK_HH3
#include <sdk/os/file.h>
#endif

namespace physik {
namespace {
constexpr const char* headers[]={"PCKNOW2 0483387abfbf9e15b57a728d\n","PCMEDIA2 7b728949c66e57f9a12819be\n"};
constexpr const char* filenames[]={"wissen.txt","bilder.dat"};
constexpr uint32_t maxRead=131072;
bool space(char ch) { return ch==' '||ch=='\t'||ch=='\r'||ch=='\n'; }
bool parse(const std::string& text,size_t& cursor,int depth,Value& result) {
  if(depth>6||cursor>=text.size())return false;
  const char tag=text[cursor++];
  const size_t begin=cursor;
  bool negative=false;
  if(cursor<text.size()&&text[cursor]=='-'){negative=true;++cursor;}
  const size_t digits=cursor;
  int64_t number=0;
  while(cursor<text.size()&&text[cursor]>='0'&&text[cursor]<='9'){
    if(cursor-begin>=10)return false;
    number=number*10+(text[cursor++]-'0');
  }
  if(cursor==digits||cursor>=text.size()||text[cursor++]!=':')return false;
  if(negative)number=-number;
  if(tag=='i'){result.type=Value::Integer;result.number=number;return true;}
  if(number<0)return false;
  if(tag=='s'){
    if(static_cast<uint64_t>(number)>text.size()-cursor)return false;
    result.type=Value::Text;
    result.text=text.substr(cursor,static_cast<size_t>(number));
    cursor+=static_cast<size_t>(number);
    return std::all_of(result.text.begin(),result.text.end(),[](unsigned char c){return c>0&&c<128;});
  }
  if(tag!='t'||number>32)return false;
  result.type=Value::Tuple;
  result.items.resize(static_cast<size_t>(number));
  for(auto& item:result.items)if(!parse(text,cursor,depth+1,item))return false;
  return true;
}
bool tuple(const Value& value,size_t length){return value.type==Value::Tuple&&value.items.size()==length;}
bool integer(const Value& value,uint32_t& result){
  if(value.type!=Value::Integer||value.number<0||value.number>UINT32_MAX)return false;
  result=static_cast<uint32_t>(value.number);return true;
}
class File {
public:
  ~File(){close();}
  bool open(const std::string& path){
    close();size=0;sizeKnown=false;error_.clear();
#ifdef PHYSIK_HH3
    if(!File_Open||!File_Close||!File_Read){error_="Dateischnittstelle nicht verfuegbar";return false;}
    path_=path;
    handle_=File_Open(path_.c_str(),FILE_OPEN_READ);
    if((handle_==FILE_ENOPATH||handle_==FILE_ENOENT||handle_==FILE_EINVAL||handle_==FILE_EBADFSID||handle_==FILE_ENOVOLUME)
       &&path.rfind("\\fls0\\",0)==0){
      // Older SDK examples use a second leading separator. Retry only that
      // documented spelling, and only after a path-related open failure.
      path_="\\"+path;
      handle_=File_Open(path_.c_str(),FILE_OPEN_READ);
    }
    if(handle_<0)return fail("Datei oeffnen",handle_);
    // File_Fstat is not required: optional seek probes may fail on a port.
    // A failed probe can move the cursor, so reset or reopen before reading.
    if(File_Lseek){
      const int end=File_Lseek(handle_,0,FILE_SEEK_END);
      if(end>0&&File_Lseek(handle_,0,FILE_SEEK_CUR)==end){size=static_cast<uint32_t>(end);sizeKnown=true;}
      if(File_Lseek(handle_,0,FILE_SEEK_SET)!=0||File_Lseek(handle_,0,FILE_SEEK_CUR)!=0){
        close();handle_=File_Open(path_.c_str(),FILE_OPEN_READ);
        if(handle_<0)return fail("Datei erneut oeffnen",handle_);
      }
    }
#else
    handle_=std::fopen(path.c_str(),"rb");
    if(!handle_)return fail("Datei oeffnen",errno);
    if(std::fseek(handle_,0,SEEK_END)!=0)return fail("Dateigroesse",errno);
    const long end=std::ftell(handle_);
    if(end<0||static_cast<uint64_t>(end)>INT_MAX||std::fseek(handle_,0,SEEK_SET)!=0)return fail("Dateigroesse",errno);
    size=static_cast<uint32_t>(end);sizeKnown=true;
#endif
    return true;
  }
  void close(){
#ifdef PHYSIK_HH3
    if(handle_>=0)File_Close(handle_);
    handle_=-1;
#else
    if(handle_)std::fclose(handle_);
    handle_=nullptr;
#endif
  }
  bool seek(uint32_t offset){
    if(offset>INT_MAX||(sizeKnown&&offset>size))return false;
#ifdef PHYSIK_HH3
    return File_Lseek&&File_Lseek(handle_,static_cast<int>(offset),FILE_SEEK_SET)==static_cast<int>(offset)
        &&File_Lseek(handle_,0,FILE_SEEK_CUR)==static_cast<int>(offset);
#else
    return std::fseek(handle_,static_cast<long>(offset),SEEK_SET)==0&&std::ftell(handle_)==static_cast<long>(offset);
#endif
  }
  bool exact(void* destination,size_t count){
    auto* bytes=static_cast<uint8_t*>(destination);
    while(count){
      const int request=static_cast<int>(std::min<size_t>(4096,count));
#ifdef PHYSIK_HH3
      const int got=File_Read(handle_,bytes,request);
#else
      const int got=static_cast<int>(std::fread(bytes,1,static_cast<size_t>(request),handle_));
#endif
      if(got==0){error_="Lesen: unerwartetes Dateiende";return false;}
      if(got<0||got>request)return fail("Lesen",got);
      bytes+=got;count-=static_cast<size_t>(got);
    }
    return true;
  }
  uint32_t size=0;
  bool sizeKnown=false;
  const std::string& error()const{return error_;}
private:
  bool fail(const char* step,int code){error_=std::string(step)+": Fehlercode "+std::to_string(code);return false;}
  std::string error_;
#ifdef PHYSIK_HH3
  int handle_=-1;
  std::string path_;
#else
  std::FILE* handle_=nullptr;
#endif
};
bool header(File& file,int kind){
  char bytes[40]{};
  const size_t length=std::strlen(headers[kind]);
  return file.exact(bytes,length)&&std::memcmp(bytes,headers[kind],length)==0;
}
}

bool decodeRecord(const std::string& text,Value& result,std::string& error){
  result=Value{};
  size_t cursor=0;
  if(!parse(text,cursor,0,result)){error="Ungueltiger Datensatz";return false;}
  while(cursor<text.size()&&space(text[cursor]))++cursor;
  if(cursor!=text.size()){error="Zusatzdaten im Datensatz";return false;}
  error.clear();return true;
}
std::string encodeRecord(const Value& value){
  if(value.type==Value::Integer)return "i"+std::to_string(value.number)+":";
  if(value.type==Value::Text)return "s"+std::to_string(value.text.size())+":"+value.text;
  std::string result="t"+std::to_string(value.items.size())+":";
  for(const auto& item:value.items)result+=encodeRecord(item);
  return result;
}
bool Store::read(int kind,uint32_t offset,uint32_t length,std::vector<uint8_t>& result){
  result.clear();error_.clear();
  if(kind<0||kind>1||length>maxRead||offset<std::strlen(headers[kind])||offset>INT_MAX||length>INT_MAX-offset){
    error_="Ungueltiger Datenbereich";return false;
  }
  const std::string path=directory_+filenames[kind];
  File file;
  if(!file.open(path)){error_=path+"\n"+file.error();return false;}
  if(file.sizeKnown&&(offset>file.size||length>file.size-offset)){error_=path+"\nDatendatei ist verkuerzt ("+std::to_string(file.size)+" Bytes)";return false;}
  if(!header(file,kind)){error_=path+"\n"+(file.error().empty()?"Paketkennung passt nicht zur App":file.error());return false;}
  if(!file.seek(offset)){
    if(!file.open(path)||!header(file,kind)){error_=path+"\nDateiposition nicht erreichbar\n"+file.error();return false;}
    uint8_t discard[4096];
    uint32_t remaining=offset-static_cast<uint32_t>(std::strlen(headers[kind]));
    while(remaining){const uint32_t count=std::min<uint32_t>(sizeof(discard),remaining);
      if(!file.exact(discard,count)){error_=path+"\nDateiposition nicht erreichbar\n"+file.error();return false;}
      remaining-=count;
    }
  }
  result.resize(length);
  if(!file.exact(result.data(),length)){result.clear();error_=path+"\n"+file.error();return false;}
  return true;
}
bool Store::row(Table table,uint32_t index,Value& result){
  if(index>=table.count||table.width>4096||static_cast<uint64_t>(table.start)+static_cast<uint64_t>(index)*table.width>UINT32_MAX){
    error_="Index ausserhalb des Verzeichnisses";return false;
  }
  std::vector<uint8_t> bytes;
  if(!read(table.file,table.start+index*table.width,table.width,bytes))return false;
  return decodeRecord(std::string(bytes.begin(),bytes.end()),result,error_);
}
bool Store::rows(Table table,uint32_t start,uint32_t count,std::vector<Value>& result){
  result.clear();
  const uint64_t offset=static_cast<uint64_t>(table.start)+static_cast<uint64_t>(start)*table.width;
  const uint64_t length=static_cast<uint64_t>(count)*table.width;
  if(start>table.count||count>table.count-start||table.width==0||table.width>4096||offset>INT_MAX||length>maxRead){error_="Ungueltiger Menuebereich";return false;}
  std::vector<uint8_t> bytes;
  if(!read(table.file,static_cast<uint32_t>(offset),static_cast<uint32_t>(length),bytes))return false;
  result.resize(count);
  for(uint32_t i=0;i<count;++i){const auto begin=bytes.begin()+i*table.width;
    if(!decodeRecord(std::string(begin,begin+table.width),result[i],error_)){result.clear();return false;}
  }
  return true;
}
bool Store::note(const std::string& id,std::string& result){
  result.clear();std::vector<Value> values;
  if(!rows(calcNotes,0,calcNotes.count,values))return false;
  for(const auto& value:values){
    if(!tuple(value,2)||value.items[0].type!=Value::Text||value.items[1].type!=Value::Text){error_="Ungueltiger Geltungshinweis";return false;}
    if(value.items[0].text==id){result=value.items[1].text;break;}
  }
  return true;
}
bool Store::chapter(uint32_t index,std::string& result){
  result.clear();Value value;uint32_t start=0,length=0;
  if(!row(chapters,index,value))return false;
  if(!tuple(value,2)||!integer(value.items[0],start)||!integer(value.items[1],length)||length>65536){error_="Ungueltiger Kapitelindex";return false;}
  std::vector<uint8_t> bytes;
  if(!read(0,start,length,bytes))return false;
  if(!std::all_of(bytes.begin(),bytes.end(),[](uint8_t c){return c>0&&c<128;})){error_="Ungueltiger Kapiteltext";return false;}
  result.assign(bytes.begin(),bytes.end());return true;
}
bool Store::media(const std::string& id,Media& result){
  std::vector<uint8_t> rows;
  if(!read(1,mediaIndex.start,mediaIndex.count*mediaIndex.width,rows))return false;
  for(uint32_t index=0;index<mediaIndex.count;++index){
    Value rowValue;
    const auto begin=rows.begin()+index*mediaIndex.width;
    if(!decodeRecord(std::string(begin,begin+mediaIndex.width),rowValue,error_))return false;
    if(!tuple(rowValue,2)||rowValue.items[0].type!=Value::Text){error_="Ungueltiges Bildverzeichnis";return false;}
    if(rowValue.items[0].text!=id)continue;
    const auto& value=rowValue.items[1];
    if(!tuple(value,8)||value.items[0].type!=Value::Text||value.items[1].type!=Value::Text||value.items[7].type!=Value::Text||
       !integer(value.items[2],result.start)||!integer(value.items[3],result.length)||!integer(value.items[4],result.width)||
       !integer(value.items[5],result.height)||!integer(value.items[6],result.checksum)||result.width<1||result.width>304||
       result.height<1||result.height>320||result.length<1||result.length>maxRead||result.checksum>65535){error_="Ungueltige Bilddaten";return false;}
    result.id=id;result.title=value.items[0].text;result.kind=value.items[1].text;result.caption=value.items[7].text;
    if(result.kind!="V"&&result.kind!="P4"&&result.kind!="P8"){error_="Unbekanntes Bildformat";return false;}
    return true;
  }
  error_="Abbildung nicht gefunden";return false;
}
bool Store::images(uint32_t chapterId,std::vector<std::string>& result){
  result.clear();
  std::vector<uint8_t> rows;
  if(!read(0,articleMedia.start,articleMedia.count*articleMedia.width,rows))return false;
  for(uint32_t i=0;i<articleMedia.count;++i){
    Value value;
    const auto begin=rows.begin()+i*articleMedia.width;
    if(!decodeRecord(std::string(begin,begin+articleMedia.width),value,error_))return false;
    if(!tuple(value,2)||value.items[0].type!=Value::Integer||value.items[1].type!=Value::Tuple){error_="Ungueltige Artikelzuordnung";return false;}
    if(value.items[0].number!=chapterId)continue;
    for(const auto& item:value.items[1].items){if(item.type!=Value::Text){error_="Ungueltige Bildkennung";return false;}result.push_back(item.text);}
    return true;
  }
  return true;
}
bool Store::imageData(const Media& media,std::vector<uint8_t>& result){
  if(!read(1,media.start,media.length,result))return false;
  uint32_t checksum=0;for(uint8_t byte:result)checksum+=byte;
  if((checksum&65535)!=media.checksum){result.clear();error_="Abbildung beschaedigt";return false;}
  return true;
}
}

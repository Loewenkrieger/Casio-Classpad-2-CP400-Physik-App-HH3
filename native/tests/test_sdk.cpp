// SPDX-License-Identifier: GPL-3.0-or-later
#include "store.hpp"
#include "paths.hpp"
#include <sdk/os/file.h>
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iterator>
#include <map>

namespace {
std::map<int,std::FILE*> handles;
int nextHandle=1,opened=0,closed=0,reads=0,readLimit=4096,statCalls=0;
bool failSeek=false,lieSeek=false,failRead=false,failStat=false,failEnd=false,zeroEnd=false,failOpen=false,legacyOnly=false;
void need(bool value,const char* message){if(!value){std::cerr<<"FAIL SDK: "<<message<<'\n';std::exit(1);}}
int openFile(const char* path,int flags){
  need(flags==FILE_OPEN_READ,"read-only open");
  if(failOpen)return FILE_ENOENT;
  std::string file=path;
  // The launcher supplies one leading separator, not the SDK example's two.
  if(file.rfind("\\\\fls0\\",0)==0){if(!legacyOnly)return FILE_ENOPATH;file="../"+file.substr(7);}
  else if(file.rfind("\\fls0\\",0)==0){if(legacyOnly)return FILE_ENOPATH;file="../"+file.substr(6);}
  auto* handle=std::fopen(file.c_str(),"rb");if(!handle)return -1;
  const int id=nextHandle++;handles[id]=handle;++opened;return id;
}
enum File_Error closeFile(int fd){
  auto it=handles.find(fd);need(it!=handles.end(),"valid close");
  std::fclose(it->second);handles.erase(it);++closed;return static_cast<File_Error>(0);
}
enum File_Error statFile(int fd,struct File_Stat* info){
  ++statCalls;
  if(failStat)return static_cast<File_Error>(-1);
  auto* handle=handles.at(fd);const long position=std::ftell(handle);
  need(std::fseek(handle,0,SEEK_END)==0,"stat seek");info->fileSize=static_cast<uint32_t>(std::ftell(handle));
  need(std::fseek(handle,position,SEEK_SET)==0,"stat restore");return static_cast<File_Error>(0);
}
int seekFile(int fd,int offset,enum File_Whence whence){
  auto* handle=handles.at(fd);
  if(whence==FILE_SEEK_END&&failEnd){std::fseek(handle,100,SEEK_SET);return -11;}
  if(whence==FILE_SEEK_SET&&failSeek){std::fseek(handle,100,SEEK_SET);return -1;}
  if(whence==FILE_SEEK_SET&&lieSeek)return offset;
  if(std::fseek(handle,offset,whence)!=0)return -1;
  if(whence==FILE_SEEK_END&&zeroEnd)return 0;
  return static_cast<int>(std::ftell(handle));
}
int readFile(int fd,void* bytes,int count){
  ++reads;need(count>=0&&count<=4096,"bounded read request");
  if(failRead)return -7;
  return static_cast<int>(std::fread(bytes,1,std::min(count,readLimit),handles.at(fd)));
}
std::vector<uint8_t> expected(){std::ifstream file("build/reference/media/rings.bin",std::ios::binary);return {std::istreambuf_iterator<char>(file),{}};}
}
extern "C" {
int (*File_Open)(const char*,int)=openFile;
enum File_Error (*File_Close)(int)=closeFile;
enum File_Error (*File_Fstat)(int,struct File_Stat*)=statFile;
int (*File_Lseek)(int,int,enum File_Whence)=seekFile;
int (*File_Read)(int,void*,int)=readFile;
}
int main(){
  const auto reference=expected();need(reference.size()==97792,"reference raster");
  need(physik::dataDirectory(nullptr)=="\\fls0\\","default launcher root");
  need(physik::dataDirectory("\\fls0\\physik.hh3")=="\\fls0\\","launcher executable root");
  need(physik::dataDirectory("\\\\fls0\\physik.hh3")=="\\fls0\\","legacy prefix normalized");
  need(physik::dataDirectory("\\fls0\\physik\\physik.hh3")=="\\fls0\\physik\\","sibling data in subfolder");
  need(physik::dataDirectory("/fls0/physik/physik.hh3")=="\\fls0\\physik\\","slash-style executable path");
  need(physik::dataDirectory("physik.hh3")=="\\fls0\\","relative argv fallback");
  const std::string longPath(512,'a');need(physik::dataDirectory(longPath.c_str())=="\\fls0\\","bounded argv path");
  {physik::Store oldPath("\\\\fls0\\");std::string chapter;
    need(!oldPath.chapter(120,chapter),"original double-prefix defect reproduced");
    need(oldPath.error().find("Datei oeffnen: Fehlercode -8")!=std::string::npos,"exact path failure reported");}
  for(int mode=0;mode<10;++mode){
    readLimit=mode==1?17:4096;failSeek=mode==2||mode==9;lieSeek=mode==3;failEnd=mode==4;zeroEnd=mode==5;legacyOnly=mode>=8;
    File_Lseek=mode==6?nullptr:seekFile;
    File_Fstat=mode==7?nullptr:statFile;
    physik::Store store(physik::dataDirectory("\\fls0\\physik.hh3"));physik::Media media;std::vector<uint8_t> data;std::string chapter;
    need(store.media("rings",media),"native metadata");need(store.imageData(media,data),"native image");
    need(data==reference,"native exact image bytes");need(store.chapter(513,chapter),"native last chapter");
    need(handles.empty()&&opened==closed,"all native files closed");
  }
  failSeek=lieSeek=failEnd=zeroEnd=legacyOnly=false;File_Lseek=seekFile;File_Fstat=statFile;
  failStat=true;
  {physik::Store store("\\fls0\\");std::string chapter;
    need(store.chapter(120,chapter),"failed fstat does not reject readable file");need(statCalls==0,"no mandatory fstat");}
  failStat=false;failRead=true;
  {physik::Store store("\\fls0\\");std::string chapter;
    need(!store.chapter(120,chapter),"native read failure rejected");need(handles.empty(),"failed read closes file");
    need(store.error().find("Lesen: Fehlercode -7")!=std::string::npos,"exact read error reported");}
  failRead=false;failOpen=true;
  {physik::Store store("\\fls0\\");std::string chapter;
    need(!store.chapter(120,chapter),"missing file rejected");
    need(store.error().find("\\fls0\\wissen.txt")!=std::string::npos,"actual file path reported");
    need(store.error().find("Datei oeffnen: Fehlercode -14")!=std::string::npos,"exact missing-file error reported");}
  failOpen=false;physik::Store store("\\fls0\\");std::string chapter;
  need(store.chapter(120,chapter),"native recovery after failure");need(opened==closed,"native handle balance");
  std::cout<<"PASS SDK: launcher and legacy paths, original path defect, short reads, optional size/seek, exact errors, recovery; "<<opened<<" opens / "<<closed<<" closes; "<<reads<<" reads\n";
}

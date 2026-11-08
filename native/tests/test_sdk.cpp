// SPDX-License-Identifier: GPL-3.0-or-later
#include "store.hpp"
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
int nextHandle=1,opened=0,closed=0,reads=0,readLimit=4096;
bool failSeek=false,lieSeek=false,failRead=false,failStat=false;
void need(bool value,const char* message){if(!value){std::cerr<<"FAIL SDK: "<<message<<'\n';std::exit(1);}}
int openFile(const char* path,int flags){
  need(flags==FILE_OPEN_READ,"read-only open");
  std::string file=path;
  if(file.rfind("\\\\fls0\\",0)==0)file="../"+file.substr(7);
  auto* handle=std::fopen(file.c_str(),"rb");if(!handle)return -1;
  const int id=nextHandle++;handles[id]=handle;++opened;return id;
}
enum File_Error closeFile(int fd){
  auto it=handles.find(fd);need(it!=handles.end(),"valid close");
  std::fclose(it->second);handles.erase(it);++closed;return static_cast<File_Error>(0);
}
enum File_Error statFile(int fd,struct File_Stat* info){
  if(failStat)return static_cast<File_Error>(-1);
  auto* handle=handles.at(fd);const long position=std::ftell(handle);
  need(std::fseek(handle,0,SEEK_END)==0,"stat seek");info->fileSize=static_cast<uint32_t>(std::ftell(handle));
  need(std::fseek(handle,position,SEEK_SET)==0,"stat restore");return static_cast<File_Error>(0);
}
int seekFile(int fd,int offset,enum File_Whence whence){
  auto* handle=handles.at(fd);
  if(whence==FILE_SEEK_SET&&failSeek){std::fseek(handle,100,SEEK_SET);return -1;}
  if(whence==FILE_SEEK_SET&&lieSeek)return offset;
  if(std::fseek(handle,offset,whence)!=0)return -1;
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
  for(int mode=0;mode<4;++mode){
    readLimit=mode==1?17:4096;failSeek=mode==2;lieSeek=mode==3;
    physik::Store store("\\\\fls0\\");physik::Media media;std::vector<uint8_t> data;std::string chapter;
    need(store.media("rings",media),"native metadata");need(store.imageData(media,data),"native image");
    need(data==reference,"native exact image bytes");need(store.chapter(513,chapter),"native last chapter");
    need(handles.empty()&&opened==closed,"all native files closed");
  }
  failSeek=lieSeek=false;
  for(int mode=0;mode<2;++mode){
    failRead=mode==0;failStat=mode==1;physik::Store store("\\\\fls0\\");std::string chapter;
    need(!store.chapter(120,chapter),"native I/O failure rejected");need(handles.empty(),"failed I/O closes file");
  }
  failRead=failStat=false;physik::Store store("\\\\fls0\\");std::string chapter;
  need(store.chapter(120,chapter),"native recovery after failure");need(opened==closed,"native handle balance");
  std::cout<<"PASS SDK: normal and short reads, failed/misreported seek, read/stat failures, recovery; "<<opened<<" opens / "<<closed<<" closes; "<<reads<<" reads\n";
}

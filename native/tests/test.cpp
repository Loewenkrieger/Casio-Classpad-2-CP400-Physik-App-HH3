// SPDX-License-Identifier: GPL-3.0-or-later
#include "app.hpp"
#include "screen_backup.hpp"
#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>

namespace {
int checks=0;
void require(bool value,const std::string& message){++checks;if(!value){std::cerr<<"FAIL: "<<message<<'\n';std::exit(1);}}
std::string readFile(const std::string& path){std::ifstream file(path,std::ios::binary);require(file.good(),"open "+path);return {std::istreambuf_iterator<char>(file),{}};}
uint32_t u32(std::ifstream& stream){uint8_t bytes[4]{};stream.read(reinterpret_cast<char*>(bytes),4);require(stream.good(),"fixture header");return (uint32_t(bytes[0])<<24)|(uint32_t(bytes[1])<<16)|(uint32_t(bytes[2])<<8)|bytes[3];}
std::string next(std::ifstream& stream){const uint32_t length=u32(stream);require(length<131072,"fixture bounds");std::string result(length,'\0');stream.read(result.data(),length);require(stream.good(),"fixture bytes");return result;}
std::vector<uint8_t> rgb(const std::vector<uint16_t>& frame,int x,int y,int w,int h){
  std::vector<uint8_t> result;result.reserve(static_cast<size_t>(w)*h*3);
  for(int row=0;row<h;++row)for(int col=0;col<w;++col){const uint16_t word=frame[(y+row)*320+x+col];
    result.push_back(((word>>11)&31)*255/31);result.push_back(((word>>5)&63)*255/63);result.push_back((word&31)*255/31);}
  return result;
}
void snapshot(const std::string& name,const std::vector<uint16_t>& frame){
  std::ofstream output("build/preview/"+name+".ppm",std::ios::binary);output<<"P6\n320 528\n255\n";
  const auto pixels=rgb(frame,0,0,320,528);output.write(reinterpret_cast<const char*>(pixels.data()),pixels.size());require(output.good(),"snapshot "+name);
}
void tap(physik::App& app,int x,int y){app.touchDown(x,y);app.touchUp();}
}
int main(){
  using namespace physik;
  std::filesystem::create_directories("build/preview");
  Store store("../");
  std::ifstream recordFixtures("build/reference/records.ref",std::ios::binary);
  uint32_t expectedRows=u32(recordFixtures),rows=0;
  for(Table table:tables)for(uint32_t i=0;i<table.count;++i){Value value;require(store.row(table,i,value),store.error());require(encodeRecord(value)==next(recordFixtures),"record equality");++rows;}
  require(rows==expectedRows,"all table rows");
  std::cout<<"Indexed records: "<<rows<<" match the Python reference\n";
  std::ifstream chapterFixtures("build/reference/chapters.ref",std::ios::binary);
  require(u32(chapterFixtures)==514,"chapter count");
  for(uint32_t i=0;i<514;++i){std::string chapter;require(store.chapter(i,chapter),store.error());require(chapter==next(chapterFixtures),"chapter equality "+std::to_string(i));}
  std::cout<<"Knowledge chapters: 514 match the Python reference\n";
  std::string text;Value value;std::string error;
  require(!store.chapter(514,text),"chapter out of bounds");
  for(const std::string invalid:{"", "t33:", "s4:abc", "x1:", "i999999999999999:", "i1:garbage", "t1:t1:t1:t1:t1:t1:t1:t1:i1:"})require(!decodeRecord(invalid,value,error),"invalid record rejected");
  std::vector<uint8_t> data;
  require(!store.read(-1,0,1,data),"bad store kind");require(!store.read(0,0,1,data),"header overlap");
  require(!store.read(0,UINT32_MAX,1,data),"offset overflow");require(!store.read(0,33,131073,data),"allocation bound");
  std::ifstream mediaList("build/reference/media.txt");std::string ident;int mediaCount=0,rasterCount=0;
  std::vector<uint16_t> frame(320*528,0xffff);Canvas canvas(frame.data());
  while(std::getline(mediaList,ident)){
    Media media;require(store.media(ident,media),ident+": "+store.error());require(store.imageData(media,data),ident+": "+store.error());
    const auto expected=readFile("build/reference/media/"+ident+".bin");require(std::equal(data.begin(),data.end(),expected.begin(),expected.end(),[](uint8_t a,char b){return a==static_cast<uint8_t>(b);}),"media bytes "+ident);
    std::fill(frame.begin(),frame.end(),0xffff);require(drawMedia(canvas,media,data,error),ident+": "+error);++mediaCount;
    if(media.kind!="V"){
      const auto pixels=rgb(frame,8,68,media.width,media.height);const auto expectedPixels=readFile("build/reference/media/"+ident+".rgb");
      require(std::equal(pixels.begin(),pixels.end(),expectedPixels.begin(),expectedPixels.end(),[](uint8_t a,char b){return a==static_cast<uint8_t>(b);}),"raster pixels "+ident);++rasterCount;
    }
    snapshot("media_"+ident,frame);
  }
  require(mediaCount==39&&rasterCount==3,"all media formats");
  Media missing;require(!store.media("not_present",missing),"missing image");
  Media bad;bad.kind="P8";bad.width=304;bad.height=320;
  require(!drawMedia(canvas,bad,{},error),"truncated raster rejected");
  bad.width=UINT32_MAX;require(!drawMedia(canvas,bad,{},error),"image size rejected");
  Store missingStore("build/does_not_exist/");require(!missingStore.chapter(0,text),"missing knowledge");
  std::filesystem::create_directories("build/bad");
  std::filesystem::copy_file("../wissen.txt","build/bad/wissen.txt",std::filesystem::copy_options::overwrite_existing);
  {std::ofstream output("build/bad/bilder.dat",std::ios::binary);output<<"bad header";}
  Store broken("build/bad/");require(!broken.media("rings",bad),"bad image file");require(broken.chapter(120,text),"theory survives media failure");
  require(store.media("electron_tube",bad),"checksum test metadata");
  auto corrupted=readFile("../bilder.dat");corrupted[bad.start+1]^=1;
  {std::ofstream output("build/bad/bilder.dat",std::ios::binary);output.write(corrupted.data(),corrupted.size());}
  require(broken.media("electron_tube",bad),"corrupt pixel metadata readable");
  require(!broken.imageData(bad,data)&&data.empty(),"corrupt image checksum rejected");
  App app("../");app.draw(canvas);snapshot("home",frame);
  app.key(Enter);require(app.page==App::Theory,"theory opens");app.draw(canvas);snapshot("theory",frame);
  app.key(Enter);require(app.page==App::Article,"article opens");app.draw(canvas);snapshot("article",frame);
  app.key(Down);app.key(Down);const int oldScroll=app.scroll;require(oldScroll>0,"article scrolling");
  app.key(Enter);require(app.page==App::Image,"EXE opens images");app.draw(canvas);snapshot("vector",frame);
  app.key(Right);app.draw(canvas);require(app.image==1,"next image");snapshot("rings",frame);
  for(int i=0;i<20;++i){app.key(Right);app.draw(canvas);require(app.page==App::Image,"twenty image switches");}
  app.key(Back);require(app.page==App::Article&&app.scroll==oldScroll,"article position preserved");
  app.key(Back);app.key(Down);app.key(Enter);app.key(Enter);app.key(Right);app.draw(canvas);snapshot("events",frame);
  require(app.page==App::Image&&app.image==1,"P4 article navigation");
  app.key(Back);app.key(Back);app.key(Back);require(app.page==App::Home,"return home");
  for(int i=0;i<3;++i)app.key(Down);
  app.key(Enter);require(app.page==App::Message,"unported calculator labelled");app.key(Back);
  app.key(Down);app.key(Enter);require(!app.running,"Beenden closes app");
  App touch("../");tap(touch,40,78);tap(touch,40,78);require(touch.page==App::Article,"touch article");
  touch.touchDown(100,250);touch.touchMove(100,180);touch.touchUp();require(touch.scroll>0,"touch scroll");
  tap(touch,260,505);require(touch.page==App::Image,"touch image button");touch.draw(canvas);tap(touch,155,505);require(touch.image==1,"touch next image");
  tap(touch,260,505);require(touch.page==App::Article,"touch return");tap(touch,15,20);tap(touch,15,20);touch.key(Clear);require(!touch.running,"Clear closes home");
  App corrupt("build/bad/");corrupt.key(Enter);corrupt.key(Enter);corrupt.key(Enter);corrupt.draw(canvas);
  require(corrupt.page==App::Message,"media failure shown");corrupt.key(Back);require(corrupt.page==App::Article,"media error returns to theory");
  std::vector<uint16_t> guarded(320*528+2,0x1357);Canvas bounds(guarded.data()+1);
  bounds.rect(-20,-20,400,600,0);bounds.line(-100,-100,420,628,0xffff);bounds.text(-10,-10,"clipping",0xffff);
  require(guarded.front()==0x1357&&guarded.back()==0x1357,"framebuffer bounds");
  ScreenBackup backup;require(!backup.save(nullptr,std::malloc,std::free),"screen null rejected");
  require(backup.save(frame.data(),std::malloc,std::free),"screen backup allocated");const auto saved=frame;
  std::fill(frame.begin(),frame.end(),0);require(backup.restore()&&frame==saved,"screen restored");require(!backup.restore(),"no duplicate release");
  std::cout<<"PASS: "<<checks<<" checks; "<<rows<<" records; 514 chapters; 39 media; 3 exact rasters; 20 image switches\n";
}

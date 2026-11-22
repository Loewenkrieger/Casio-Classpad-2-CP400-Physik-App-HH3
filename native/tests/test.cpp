// SPDX-License-Identifier: GPL-3.0-or-later
#include "app.hpp"
#include "screen_backup.hpp"
#include "knowledge.hpp"
#include "browse_helpers.hpp"
#include <algorithm>
#include <cstdlib>
#include <cstdio>
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
void choose(physik::App& app,int index){
  const auto labels=app.labels();require(index>=0&&index<static_cast<int>(labels.size()),"menu index");
  const int delta=index-app.selected;
  for(int i=0;i<std::abs(delta);++i)app.key(delta<0?physik::Up:physik::Down);
  require(app.selected==index,"menu selection");app.key(physik::Enter);
}
void choose(physik::App& app,const std::string& label){
  const auto labels=app.labels();const auto found=std::find(labels.begin(),labels.end(),label);
  require(found!=labels.end(),"menu label "+label);choose(app,static_cast<int>(found-labels.begin()));
}
void home(physik::App& app){
  for(int i=0;i<8&&app.page!=physik::App::Home;++i)app.key(physik::Back);
  require(app.running&&app.page==physik::App::Home,"return home");
}
void type(physik::App& app,const std::string& value){for(char key:value)app.key(key);}
void number(physik::App& app,double value){char text[48]{};std::snprintf(text,sizeof(text),"%.17g",value);type(app,text);app.key(physik::Enter);}
void quantumArticle(physik::App& app,const std::string& title){
  const uint32_t chapter=title=="De Broglie und Modellgrenzen"?120:123;
  home(app);choose(app,0);std::vector<int> path;std::string error;
  require(browse_test::find(app.store,physik::navigationRoot(0),physik::NavigationKind::LegacyArticle,chapter,path,error,0,11),"quantum path: "+error);
  for(int index:path)choose(app,index);
  require(app.page==physik::App::Article,"quantum article");
}
}
int main(){
  using namespace physik;
  std::filesystem::create_directories("build/preview");
  Store store("../");
  std::ifstream recordFixtures("build/reference/records.ref",std::ios::binary);
  uint32_t expectedRows=u32(recordFixtures),rows=0;
  for(Table table:tables){
    std::vector<Value> batch;require(store.rows(table,0,table.count,batch),store.error());require(batch.size()==table.count,"batch count");
    for(uint32_t i=0;i<table.count;++i){Value value;require(store.row(table,i,value),store.error());
      require(encodeRecord(value)==next(recordFixtures),"record equality");require(encodeRecord(batch[i])==encodeRecord(value),"batch row equality");++rows;}
  }
  require(rows==expectedRows,"all table rows");
  std::cout<<"Indexed records: "<<rows<<" match the Python reference\n";
  std::ifstream chapterFixtures("build/reference/chapters.ref",std::ios::binary);
  require(u32(chapterFixtures)==514,"chapter count");
  for(uint32_t i=0;i<514;++i){std::string chapter;require(store.chapter(i,chapter),store.error());require(chapter==next(chapterFixtures),"chapter equality "+std::to_string(i));}
  std::cout<<"Knowledge chapters: 514 match the Python reference\n";
  std::string text;Value value;std::string error;
  std::vector<Value> invalidRows;
  require(!store.rows(entries,entries.count,1,invalidRows),"batch past table");
  require(!store.rows(entries,UINT32_MAX,1,invalidRows),"batch start overflow");
  require(!store.rows(Table{0,1,UINT32_MAX,72},0,1,invalidRows),"batch offset overflow");
  require(!store.rows(Table{0,UINT32_MAX,33,4096},0,UINT32_MAX,invalidRows),"batch allocation limit");
  require(!store.rows(Table{0,1,33,0},0,1,invalidRows),"batch zero width");
  for(uint32_t i:{0u,calcNotes.count-1}){Value note;require(store.row(calcNotes,i,note),"note reference");
    require(store.note(note.items[0].text,text)&&text==note.items[1].text,"calculator note equality");}
  require(store.note("not_present",text)&&text.empty(),"absent calculator note");
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
  std::filesystem::copy_file("build/package/wissen.txt","build/bad/wissen.txt",std::filesystem::copy_options::overwrite_existing);
  {std::ofstream output("build/bad/bilder.dat",std::ios::binary);output<<"bad header";}
  Store broken("build/bad/");require(!broken.media("rings",bad),"bad image file");require(broken.chapter(120,text),"theory survives media failure");
  require(store.media("electron_tube",bad),"checksum test metadata");
  auto corrupted=readFile("../bilder.dat");corrupted[bad.start+1]^=1;
  {std::ofstream output("build/bad/bilder.dat",std::ios::binary);output.write(corrupted.data(),corrupted.size());}
  require(broken.media("electron_tube",bad),"corrupt pixel metadata readable");
  require(!broken.imageData(bad,data)&&data.empty(),"corrupt image checksum rejected");
  App browse("build/package/");int entryCount=0,calcCount=0;
  for(int mode=0;mode<3;++mode){
    choose(browse,mode);require(browse.page==App::Theory,"search root");
    const auto branches=browse.labels().size();require(mode==App::AlphabetSearch||branches==15,"teacher topic roots unchanged");
    for(size_t topic=0;topic<branches;++topic){
      choose(browse,topic);require(browse.page==App::Theory,"topic subgroups");const auto count=browse.labels().size();
      for(size_t group=0;group<count;++group){
        choose(browse,group);require(browse.page==App::Entries,"group entries");const auto leaves=browse.labels().size();
        for(size_t entry=0;entry<leaves;++entry){choose(browse,entry);require(browse.page==App::Article,"all article links");++entryCount;browse.key(Back);}
        browse.key(Back);
      }
      browse.key(Back);
    }
    browse.key(Back);
  }
  require(entryCount>=978,"all existing bodies remain reachable through the hierarchy");
  choose(browse,App::CalculatorMenu);require(browse.labels().size()==18+nativeCategories().size(),"legacy categories plus native extensions");
  for(int category=0;category<17;++category){
    choose(browse,category);require(browse.page==App::Calcs,"calculator category");const auto count=browse.labels().size();
    for(size_t calc=0;calc<count;++calc){choose(browse,calc);require(browse.page==App::Form,"all calculator forms");++calcCount;browse.key(Back);}
    browse.key(Back);
  }
  require(calcCount==167,"all 167 calculators reachable");
  choose(browse,"Kinematik: beschleunigt");require(browse.page==App::Calcs&&browse.labels().size()==7,"native catalog opens without disk indices");
  for(int i=0;i<7;++i){choose(browse,i);require(browse.page==App::Form,"native kinematics form");browse.key(Back);}
  App app("build/package/");app.draw(canvas);snapshot("home",frame);
  choose(app,0);require(app.page==App::Theory,"theory opens");app.draw(canvas);snapshot("theory",frame);
  choose(app,11);app.draw(canvas);snapshot("entries",frame);
  quantumArticle(app,"De Broglie und Modellgrenzen");app.draw(canvas);snapshot("article",frame);
  app.key(Down);app.key(Down);const int oldScroll=app.scroll;require(oldScroll>0,"article scrolling");
  app.key(Enter);require(app.page==App::Image,"EXE opens images");app.draw(canvas);snapshot("vector",frame);
  app.key(Right);app.draw(canvas);require(app.image==1,"next image");snapshot("rings",frame);
  for(int i=0;i<20;++i){app.key(Right);app.draw(canvas);require(app.page==App::Image,"twenty image switches");}
  app.key(Back);require(app.page==App::Article&&app.scroll==oldScroll,"article position preserved");
  quantumArticle(app,"Doppelspalt mit Quantenobjekten");app.key(Enter);app.key(Right);app.draw(canvas);snapshot("events",frame);
  require(app.page==App::Image&&app.image==1,"P4 article navigation");
  home(app);choose(app,App::CalculatorMenu);choose(app,0);choose(app,0);type(app,"2");app.key(Enter);
  require(app.page==App::Message&&app.messageText().find("T = 0.5 s")!=std::string::npos,"period UI result");app.draw(canvas);snapshot("period_result",frame);
  app.key(Back);type(app,"4");app.key(Enter);require(app.messageText().find("T = 0.25 s")!=std::string::npos,"replace prior input");
  app.key(Back);app.key('E');app.key(Enter);require(app.messageText().find("T = 0.25 s")!=std::string::npos,"invalid initial exponent preserves prior input");
  app.key(Back);type(app,"0");app.key(Enter);require(app.page==App::Message&&app.messageText().find("0.25")==std::string::npos,"zero denominator message");
  app.key(Back);tap(app,115,386);tap(app,193,444);require(app.messageText().find("T = 0.5 s")!=std::string::npos,"touch number and OK");
  home(app);choose(app,App::CalculatorMenu);choose(app,8);choose(app,1);type(app,"500");app.draw(canvas);snapshot("photon_form",frame);app.key(Enter);
  require(app.page==App::Message&&app.messageText().find("2.47968 eV")!=std::string::npos,"500 nm UI result");app.draw(canvas);snapshot("photon_result",frame);
  app.key(Back);tap(app,270,444);app.key(Enter);require(app.messageText().find("2479.68 eV")!=std::string::npos,"unit change to pm");
  home(app);choose(app,App::CalculatorMenu);choose(app,2);choose(app,0);type(app,"1");app.key(Enter);require(app.field==1,"advance to next input");
  app.key(Left);require(app.field==0,"return from empty next field");
  for(int workflow=0;workflow<4;++workflow){
    home(app);choose(app,App::CalculatorMenu);choose(app,16);choose(app,workflow);
    const bool wavelength=workflow>=2,variable=workflow%2==1;const int count=variable?3:2;
    if(variable)number(app,count);
    for(int i=0;i<count;++i){const double frequency=(600+i*100)*1e12;
      number(app,wavelength?light/frequency*1e9:frequency/1e12);number(app,planck*frequency/charge-2);}
    require(app.page==App::Message&&app.messageText().find("6.62607e-34")!=std::string::npos&&app.messageText().find("W_A")!=std::string::npos,"Planck UI workflow "+std::to_string(workflow));
    if(workflow==3){app.draw(canvas);snapshot("fit_result",frame);}
  }
  home(app);choose(app,App::CalculatorMenu);choose(app,"Kinematik: beschleunigt");app.draw(canvas);snapshot("kinematics_menu",frame);
  choose(app,0);number(app,0);number(app,0);number(app,1.8);type(app,"5");app.draw(canvas);snapshot("kinematics_form",frame);app.key(Enter);
  require(app.page==App::Message&&app.messageText().find("x = 22.5 m")!=std::string::npos&&app.messageText().find("v = 9 m/s")!=std::string::npos&&app.messageText().find("Weg = 22.5 m")!=std::string::npos,"Bob multiple named results");
  app.draw(canvas);snapshot("kinematics_bob",frame);app.key(Back);type(app,"0");app.key(Enter);
  require(app.messageText().find("Weg = 0 m")!=std::string::npos,"zero duration is valid forward");
  home(app);choose(app,App::CalculatorMenu);choose(app,"Kinematik: beschleunigt");choose(app,1);number(app,0);number(app,10);number(app,-2);number(app,16);
  require(app.page==App::Message&&app.messageText().find("t_1 = 2 s")!=std::string::npos&&app.messageText().find("t_2 = 8 s")!=std::string::npos,"two future roots visible");
  app.draw(canvas);snapshot("kinematics_roots",frame);app.key(Back);type(app,"26");app.key(Enter);
  require(app.page==App::Message&&app.messageText().find("t_1 =")==std::string::npos,"unreachable target shown as error");
  app.key(Back);type(app,"25");app.key(Enter);require(app.messageText().find("t = 5 s")!=std::string::npos,"correct input after no-solution error");
  home(app);choose(app,App::CalculatorMenu);choose(app,"Kinematik: beschleunigt");choose(app,1);number(app,0);number(app,1);number(app,-1);number(app,.49999999999999645);
  require(app.page==App::Message&&app.messageText().find("t_1 = 0.9999999157 s")!=std::string::npos&&app.messageText().find("t_2 = 1.000000084 s")!=std::string::npos,"close roots use distinct displayed precision");
  app.draw(canvas);snapshot("kinematics_close_roots",frame);
  home(app);choose(app,App::CalculatorMenu);choose(app,"Kinematik: beschleunigt");choose(app,6);type(app,"54");tap(app,270,444);app.key(Enter);
  type(app,"72");app.key('U');app.key(Enter);number(app,25);
  require(app.page==App::Message&&app.messageText().find("a = 3.5 m/s^2")!=std::string::npos,"Gelbphase km/h input");
  app.draw(canvas);snapshot("kinematics_gelbphase",frame);
  home(app);choose(app,App::CalculatorMenu);choose(app,"Kinematik: beschleunigt");choose(app,0);type(app,"1");app.key('U');app.key(Enter);
  number(app,2);number(app,0);type(app,"1");app.key('U');app.key('U');app.key(Enter);
  require(app.messageText().find("x = 8200 m")!=std::string::npos,"km length and hour duration converted to SI");
  home(app);choose(app,App::CalculatorMenu);choose(app,"Kinematik: beschleunigt");choose(app,0);number(app,100);number(app,-10);number(app,2);number(app,8);
  require(app.messageText().find("Delta_x = -16 m")!=std::string::npos&&app.messageText().find("Weg = 34 m")!=std::string::npos,"reversal path differs from signed displacement");
  for(int i=0;i<20;++i){app.key(Back);type(app,i%2?"8":"7");app.key(Enter);require(app.page==App::Message,"repeated kinematics correction and compute");}
  quantumArticle(app,"De Broglie und Modellgrenzen");app.key(Enter);app.key(Right);app.draw(canvas);
  require(app.page==App::Image,"images still render after repeated kinematics calculations");
  home(app);choose(app,App::CalculatorMenu);choose(app,8);choose(app,1);number(app,500);
  require(app.messageText().find("2.47968 eV")!=std::string::npos,"legacy quantum default units restored after native masks");
  home(app);choose(app,App::ExitMenu);require(!app.running,"Beenden closes app");
  App touch("build/package/");quantumArticle(touch,"De Broglie und Modellgrenzen");
  touch.touchDown(100,250);touch.touchMove(100,180);touch.touchUp();require(touch.scroll>0,"touch scroll");
  tap(touch,260,505);require(touch.page==App::Image,"touch image button");touch.draw(canvas);tap(touch,155,505);require(touch.image==1,"touch next image");
  tap(touch,260,505);require(touch.page==App::Article,"touch return");tap(touch,15,20);require(touch.page==App::Entries,"touch back to entries");
  home(touch);tap(touch,40,78);touch.touchDown(100,350);touch.touchMove(100,120);touch.touchUp();require(touch.selected>0,"touch menu scrolling");
  home(touch);touch.key(Clear);require(!touch.running,"Clear closes home");
  App corrupt("build/bad/");quantumArticle(corrupt,"De Broglie und Modellgrenzen");corrupt.key(Enter);corrupt.draw(canvas);
  require(corrupt.page==App::Message,"media failure shown");corrupt.key(Back);require(corrupt.page==App::Article,"media error returns to theory");
  std::vector<uint16_t> guarded(320*528+2,0x1357);Canvas bounds(guarded.data()+1);
  bounds.rect(-20,-20,400,600,0);bounds.line(-100,-100,420,628,0xffff);bounds.text(-10,-10,"clipping",0xffff);
  require(guarded.front()==0x1357&&guarded.back()==0x1357,"framebuffer bounds");
  ScreenBackup backup;require(!backup.save(nullptr,std::malloc,std::free),"screen null rejected");
  require(backup.save(frame.data(),std::malloc,std::free),"screen backup allocated");const auto saved=frame;
  std::fill(frame.begin(),frame.end(),0);require(backup.restore()&&frame==saved,"screen restored");require(!backup.restore(),"no duplicate release");
  std::cout<<"PASS: "<<checks<<" checks; "<<rows<<" records; 514 chapters; 39 media; 3 exact rasters; 20 image switches; "<<entryCount<<" browse entries; "<<calcCount<<" calculators; 7 native kinematics forms\n";
}

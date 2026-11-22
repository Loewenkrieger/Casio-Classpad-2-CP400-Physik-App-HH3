// SPDX-License-Identifier: GPL-3.0-or-later
#include "app.hpp"
#include "knowledge.hpp"
#include "browse_helpers.hpp"
#include <algorithm>
#include <array>
#include <cctype>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <set>

namespace {
using namespace physik;
#include "navigation_index.inc"
unsigned checks=0;
using Ref=std::pair<uint32_t,uint32_t>;
using Refs=std::set<Ref>;
const char* topics[]={"00 Kinematik","01 Erhaltungssaetze","02 Kraefte","03 Bewegung / Wurf","04 Elektrische Felder",
  "05 Magnetisches Feld","06 Teilchen in Feldern","07 Elektromagn. Induktion","08 Modellbildung / Simulation",
  "09 Schwingungen","10 Wellen","11 Quantenphysik","12 Grundlagen Atomphysik","Abiturvorbereitung","Sonstiges"};
void require(bool ok,const std::string& why){++checks;if(!ok){std::cerr<<"FAIL NAVIGATION: "<<why<<'\n';std::exit(1);}}
std::string read(const std::string& path){std::ifstream in(path,std::ios::binary);require(in.good(),"read "+path);return {std::istreambuf_iterator<char>(in),{}};}
void write(const std::string& data){std::ofstream out("build/navigation_bad/wissen.txt",std::ios::binary);out.write(data.data(),data.size());require(out.good(),"fixture write");}
uint32_t hash(const std::string& data){uint32_t h=2166136261u;for(unsigned char c:data)h=(h^c)*16777619u;return h;}
Value decoded(const std::string& data,size_t start,size_t width){Value out;std::string error;require(start+width<=data.size(),"record bounds");require(decodeRecord(data.substr(start,width),out,error),error);return out;}
void record(std::string& data,size_t start,size_t width,const Value& value){auto text=encodeRecord(value);require(text.size()<=width,"record fits");text.resize(width,' ');data.replace(start,width,text);}
uint32_t metaHash(const Value& v){std::string bytes;for(unsigned i=0;i<4;++i)for(int shift=24;shift>=0;shift-=8)bytes.push_back((static_cast<uint32_t>(v.items[i].number)>>shift)&255);return hash(bytes);}
void changeChild(std::string& data,uint32_t node,size_t index,const Value& replacement){
  auto directory=decoded(data,navigationDirectory.start+node*navigationDirectory.width,navigationDirectory.width);
  const auto start=directory.items[0].number,count=directory.items[1].number,width=directory.items[2].number;
  require(index<static_cast<size_t>(count),"child index");record(data,start+index*width,width,replacement);
  directory.items[3].number=hash(data.substr(start,count*width));directory.items[4].number=metaHash(directory);
  record(data,navigationDirectory.start+node*navigationDirectory.width,navigationDirectory.width,directory);
}
Value child(const std::string& data,uint32_t node,size_t index){const auto d=decoded(data,navigationDirectory.start+node*navigationDirectory.width,navigationDirectory.width);return decoded(data,d.items[0].number+index*d.items[2].number,d.items[2].number);}
void rejected(Store& store,const std::string& data,uint32_t node){write(data);std::vector<NavigationItem> rows{{"stale",NavigationKind::Menu,3}};std::string error="old";
  require(!navigationRows(store,node,rows,error)&&rows.empty()&&!error.empty(),"invalid navigation rejected atomically");}
void focus(App& app,size_t index){const auto labels=app.labels();require(index<labels.size(),"menu index");while(app.selected<static_cast<int>(index))app.key(Down);while(app.selected>static_cast<int>(index))app.key(Up);}
void choose(App& app,size_t index){focus(app,index);app.key(Enter);}
void home(App& app){for(unsigned i=0;i<10&&app.page!=App::Home;++i)app.key(Back);require(app.running&&app.page==App::Home,"home");}
void tap(App& app,int x,int y){app.touchDown(x,y);app.touchUp();}
std::vector<uint16_t> render(App& app){std::vector<uint16_t> result(320*528);Canvas c(result.data());app.draw(c);return result;}
void snapshot(const std::string& name,App& app){const auto pixels=render(app);std::ofstream out("build/preview/nav_"+name+".ppm",std::ios::binary);out<<"P6\n320 528\n255\n";
  for(uint16_t p:pixels){out.put(((p>>11)&31)*255/31);out.put(((p>>5)&63)*255/63);out.put((p&31)*255/31);}require(out.good(),"snapshot");}
void walk(Store& store,uint32_t node,unsigned mode,unsigned depth,int topic,Refs& found,std::array<Refs,15>& byTopic,std::set<uint32_t>& nodes,size_t& links){
  nodes.insert(node);require(depth<=2,"exactly three menu levels");std::vector<NavigationItem> rows;std::string error;
  require(navigationRows(store,node,rows,error),error);require(!rows.empty(),"no empty list");
  if(depth==0){require(rows.size()==15,"all teacher topics");for(size_t i=0;i<15;++i)require(rows[i].label==topics[i],"teacher order and label");}
  if(depth==1&&mode==1)for(const auto& row:rows)require(row.label.find(" - ")!=std::string::npos,"quantity has symbol and clear name: "+row.label);
  for(size_t i=0;i<rows.size();++i){const auto& row=rows[i];
    require(row.label.rfind("Neu:",0)!=0,"old and new groups merged");
    uint16_t pixel=0;Canvas measure(&pixel);
    require(measure.width(row.label)<=296||measure.wrap(row.label,296,true).size()<=2,"full navigation label fits two lines");
    if(depth<2){require(row.kind==NavigationKind::Menu,"intermediate menu type");walk(store,row.target,mode,depth+1,depth==0?static_cast<int>(i):topic,found,byTopic,nodes,links);}
    else {require(row.kind!=NavigationKind::Menu,"article leaf type");const Ref ref{static_cast<uint32_t>(row.kind),row.target};found.insert(ref);byTopic[topic].insert(ref);++links;
      require(row.kind==NavigationKind::NativeArticle||(mode==0?row.target<146:row.target>=146),"correct content type for mode");}
  }
}
std::string lower(std::string text){for(char& c:text)c=static_cast<char>(std::tolower(static_cast<unsigned char>(c)));return text;}
size_t indexOf(const std::vector<NavigationItem>& rows,const std::string& name){
  const auto it=std::find_if(rows.begin(),rows.end(),[&](const auto& row){return row.label==name;});
  require(it!=rows.end(),"alphabet entry "+name);return it-rows.begin();
}
void alphabetWalk(Store& store,std::set<uint32_t>& nodes,size_t& links){
  Refs found;std::set<std::string> quantities;std::vector<NavigationItem> letters;std::string error;
  const auto root=navigationRoot(3);require(root==navigationAlphabetRoot,"alphabet root mapping");nodes.insert(root);
  require(navigationRows(store,root,letters,error),error);std::string previousLetter;
  for(const auto& letter:letters){
    require(letter.kind==NavigationKind::Menu&&letter.label.size()==1&&letter.label>previousLetter&&letter.label>="A"&&letter.label<="Z","ordered nonempty letter menus");
    previousLetter=letter.label;nodes.insert(letter.target);std::vector<NavigationItem> groups;
    require(navigationRows(store,letter.target,groups,error),error);std::string previousQuantity;
    for(const auto& group:groups){
      const auto normalized=lower(group.label);
      require(group.kind==NavigationKind::Menu&&group.label.find(" - ")!=std::string::npos&&normalized>previousQuantity,"quantities sorted ignoring letter case");
      require(lower(group.label.substr(0,1))==lower(letter.label),"quantity in correct initial");previousQuantity=normalized;
      require(quantities.insert(group.label).second,"quantity appears exactly once globally");nodes.insert(group.target);
      std::vector<NavigationItem> leaves;require(navigationRows(store,group.target,leaves,error),error);
      for(const auto& leaf:leaves){require(leaf.kind==NavigationKind::NativeArticle||(leaf.kind==NavigationKind::LegacyArticle&&leaf.target>=146),"alphabet contains formulas or supplemental articles only");
        found.insert({static_cast<uint32_t>(leaf.kind),leaf.target});++links;uint16_t pixel=0;Canvas measure(&pixel);
        require(measure.width(leaf.label)<=296||measure.wrap(leaf.label,296,true).size()<=2,"alphabet formula title readable");}
    }
  }
  require(quantities.size()==142&&found.size()==400,"all 142 quantities and 400 unique formula chapters");
  for(uint32_t id=146;id<514;++id)require(found.count({1,id})==1,"all legacy formulas in alphabet");
  for(uint32_t id=0;id<32;++id)require(found.count({2,id})==1,"all supplemental articles in alphabet");
  for(const char* name:{"a - Beschleunigung","A - Flaeche","a - Leiterabstand","A_s - Schwingungsamplitude",
      "Q / q - Elektrische Ladung","Q_Guete - Schwingkreisguete","rho_m - Massendichte","rho_el - Spezifischer elektrischer Widerstand"})
    require(quantities.count(name)==1,"separate meaning: "+std::string(name));
}
}
int main(){
  using namespace physik;
  std::filesystem::create_directories("build/preview");std::filesystem::create_directories("build/navigation_bad");
  Store store("build/package/");std::set<uint32_t> nodes;Refs all;std::array<size_t,4> linkCounts{};
  std::array<std::array<Refs,15>,3> byTopic;
  for(unsigned mode=0;mode<3;++mode){Refs found;walk(store,navigationRoot(mode),mode,0,-1,found,byTopic[mode],nodes,linkCounts[mode]);
    require(found.size()==(mode==0?178:400),"unique complete mode coverage");
    for(uint32_t id=mode==0?0:146;id<(mode==0?146:514);++id)require(found.count({1,id})==1,"every frozen chapter in correct mode");
    for(uint32_t id=0;id<32;++id)require(found.count({2,id})==1,"all native articles in every mode");
    all.insert(found.begin(),found.end());
  }
  require(nodes.size()==navigationAlphabetRoot,"existing teacher trees keep their node IDs");
  alphabetWalk(store,nodes,linkCounts[3]);
  require(all.size()==546&&nodes.size()==navigationNodeCount(),"all chapters and nodes reachable");
  for(unsigned mode=1;mode<3;++mode)for(int topic:{5,6})require(byTopic[mode][topic].count({1,156})==1,"Lorentz formula in magnetic and particle topics");
  for(unsigned mode=0;mode<3;++mode)for(int topic:{7,9})require(byTopic[mode][topic].count({2,27})==1,"AC article in induction and oscillations");
  for(int topic:{11,12})require(byTopic[0][topic].count({1,134})==1,"potential well in quantum and atom theory");
  std::vector<int> powerPath;std::string error;
  require(browse_test::find(store,1,NavigationKind::LegacyArticle,491,powerPath,error,0,9),"loss power path");
  std::vector<NavigationItem> root,groups;require(navigationRows(store,1,root,error),error);
  require(navigationRows(store,root[9].target,groups,error),error);
  require(groups[powerPath[1]].label.front()=='P'&&groups[powerPath[1]].label.find("leistung")!=std::string::npos,"loss power not classified as resistance");
  const auto lossNode=groups[powerPath[1]].target;

  const auto correct=read("build/package/wissen.txt");Store bad("build/navigation_bad/");
  auto corrupted=correct;corrupted[navigationOffset+10]^=1;rejected(bad,corrupted,0);
  std::string theory;require(bad.chapter(0,theory),"legacy theory survives bad navigation");
  rejected(bad,correct.substr(0,navigationOffset),0);
  corrupted=correct;auto directory=decoded(correct,navigationDirectory.start,navigationDirectory.width);
  directory.items[1].number+=1;record(corrupted,navigationDirectory.start,navigationDirectory.width,directory);rejected(bad,corrupted,0);
  directory=decoded(correct,navigationDirectory.start,navigationDirectory.width);
  corrupted=correct;corrupted[directory.items[0].number+1]^=1;rejected(bad,corrupted,0);
  for(unsigned field:{0u,1u,2u}){corrupted=correct;directory=decoded(correct,navigationDirectory.start,navigationDirectory.width);
    directory.items[field].number=field==0?navigationOffset:field==1?0:513;directory.items[4].number=metaHash(directory);
    record(corrupted,navigationDirectory.start,navigationDirectory.width,directory);rejected(bad,corrupted,0);}
  for(uint32_t target:{0u,1u,navigationAlphabetRoot,navigationNodeCount()}){corrupted=correct;auto row=child(correct,0,0);row.items[2].number=target;changeChild(corrupted,0,0,row);rejected(bad,corrupted,0);}
  corrupted=correct;auto row=child(correct,0,1);row.items[0]=child(correct,0,0).items[0];changeChild(corrupted,0,1,row);rejected(bad,corrupted,0);
  corrupted=correct;row=child(correct,0,1);row.items[2]=child(correct,0,0).items[2];changeChild(corrupted,0,1,row);rejected(bad,corrupted,0);
  corrupted=correct;row=child(correct,0,0);row.items[1].number=99;changeChild(corrupted,0,0,row);rejected(bad,corrupted,0);
  corrupted=correct;row=child(correct,0,0);row.items[0].text.clear();changeChild(corrupted,0,0,row);rejected(bad,corrupted,0);
  corrupted=correct;row=child(correct,0,0);row.items[1].number=1;row.items[2].number=120;changeChild(corrupted,0,0,row);rejected(bad,corrupted,0);
  for(unsigned kind:{1u,2u}){corrupted=correct;row=child(correct,lossNode,0);row.items[1].number=kind;
    row.items[2].number=kind==1?chapters.count:supplementCount();changeChild(corrupted,lossNode,0,row);rejected(bad,corrupted,lossNode);}
  rejected(bad,correct.substr(0,correct.size()-1),navigationNodeCount()-1);
  std::vector<NavigationItem> invalid;require(!navigationRows(store,navigationNodeCount(),invalid,error),"node bounds");
  require(navigationRoot(4)==UINT32_MAX,"mode bounds");
  write(correct);require(navigationRows(bad,0,invalid,error),"recovery after bad file");

  App app("build/package/");
  const std::vector<std::string> expectedHome={"Theorie","Formeln nach Themen","Formeln A-Z","Physik-Rechner","Beenden"};
  require(app.labels()==expectedHome,"five home entries without redundant quantity search");
  for(size_t mode:{size_t(App::TheorySearch),size_t(App::TopicSearch)}){choose(app,mode);snapshot("root_"+std::to_string(mode),app);require(app.labels().size()==15,"UI root count");require(app.loadedNavigationLevels()==1,"only active root loaded");app.key(Back);require(app.loadedNavigationLevels()==0,"root freed on exit");}
  choose(app,0);std::vector<int> path;require(browse_test::find(store,0,NavigationKind::LegacyArticle,120,path,error,0,11),error);
  require(path.size()==3,"three-level path");
  choose(app,11);snapshot("quantum_subtopics",app);require(app.loadedNavigationLevels()==1,"root rows released");
  while(app.selected<path[1])app.key(Down);
  while(app.selected>path[1])app.key(Up);
  const auto topicFrame=render(app);app.key(Enter);
  require(app.loadedNavigationLevels()==1,"ancestor rows released");
  while(app.selected<path[2])app.key(Down);
  while(app.selected>path[2])app.key(Up);
  const auto leafFrame=render(app);snapshot("quantum_articles",app);app.key(Enter);require(app.page==App::Article,"article via hierarchy");
  app.key(Down);app.key(Down);const int position=app.scroll;app.key(Enter);require(app.page==App::Image,"image via new hierarchy");
  for(int i=0;i<20;++i){app.key(Right);render(app);require(app.page==App::Image,"images after regrouping");}
  app.key(Back);require(app.page==App::Article&&app.scroll==position,"text position preserved");app.key(Back);
  require(render(app)==leafFrame,"leaf selection and scroll restored");app.key(Back);require(render(app)==topicFrame,"subtopic selection restored");
  require(app.loadedNavigationLevels()==1,"only reloaded parent remains");
  app.key(Back);require(app.selected==11,"root selection restored");const auto rootFrame=render(app);choose(app,11);app.key(Back);require(render(app)==rootFrame,"root scroll restored");
  home(app);choose(app,App::TopicSearch);choose(app,9);snapshot("oscillation_topics",app);
  std::vector<NavigationItem> topicRoot,topicGroups;require(navigationRows(store,navigationRoot(2),topicRoot,error),error);
  require(navigationRows(store,topicRoot[9].target,topicGroups,error),error);std::vector<std::string> topicLabels;
  for(const auto& item:topicGroups)topicLabels.push_back(item.label);
  require(app.labels()==topicLabels,"topic menu still uses topic data, not quantity data");
  tap(app,15,20);require(app.page==App::Theory&&app.selected==9,"touch back restores topic");
  app.touchDown(100,350);app.touchMove(100,120);app.touchUp();require(app.selected>0,"touch scroll on topic menu");
  home(app);choose(app,App::AlphabetSearch);snapshot("alphabet_letters",app);
  std::vector<NavigationItem> letters,quantities;require(navigationRows(store,navigationRoot(3),letters,error),error);
  const auto r=indexOf(letters,"R"),l=indexOf(letters,"L");focus(app,r);const auto letterFrame=render(app);app.key(Enter);
  require(navigationRows(store,letters[r].target,quantities,error),error);const auto mass=indexOf(quantities,"rho_m - Massendichte");
  focus(app,mass);snapshot("alphabet_R",app);const auto quantityFrame=render(app);app.key(Enter);
  require(app.page==App::Entries&&app.loadedNavigationLevels()==1,"alphabet loads only active formula list");
  choose(app,0);require(app.page==App::Article,"article from alphabet");app.key(Back);app.key(Back);
  require(render(app)==quantityFrame&&app.loadedNavigationLevels()==1,"alphabet quantity selection and scroll restored");app.key(Back);
  require(render(app)==letterFrame,"alphabet letter selection and scroll restored");app.key(Back);require(app.selected==App::AlphabetSearch,"alphabet home selection restored");
  choose(app,App::AlphabetSearch);std::vector<int> imagePath;
  require(browse_test::find(store,navigationRoot(3),NavigationKind::NativeArticle,30,imagePath,error,0,l),"relativity article under L");
  for(int index:imagePath)choose(app,index);
  require(app.page==App::Article,"supplemental article from alphabet");app.key(Down);const auto textPosition=app.scroll;app.key(Enter);
  require(app.page==App::Image,"images reachable from alphabet");
  for(unsigned i=0;i<20;++i){app.key(Right);render(app);require(app.page==App::Image,"alphabet image switches");}
  app.key(Back);require(app.page==App::Article&&app.scroll==textPosition,"alphabet image restores text position");
  home(app);tap(app,40,172);require(app.page==App::Theory&&app.labels().front()=="A","touch alphabet home entry");
  tap(app,40,78);require(app.labels().front()=="a - Beschleunigung","touch letter A");
  tap(app,15,20);require(app.selected==0&&app.loadedNavigationLevels()==1,"touch back to letters");
  home(app);choose(app,App::CalculatorMenu);choose(app,0);choose(app,0);app.key('2');app.key(Enter);require(app.messageText().find("T = 0.5 s")!=std::string::npos,"old calculator unchanged");
  home(app);choose(app,App::ExitMenu);require(!app.running,"exit after navigation");
  write(correct);App changed("build/navigation_bad/");choose(changed,0);choose(changed,11);
  corrupted=correct;corrupted[navigationOffset+10]^=1;write(corrupted);changed.key(Back);
  require(changed.page==App::Message&&changed.loadedNavigationLevels()==0,"bad parent reload releases menu state");
  changed.key(Back);require(changed.page==App::Home,"failed reload cannot trap user in a broken submenu");
  write(correct);choose(changed,0);choose(changed,11);require(changed.loadedNavigationLevels()==1,"navigation recovers after restoring package");
  home(changed);
  corrupted=correct;corrupted[navigationOffset+10]^=1;write(corrupted);App broken("build/navigation_bad/");choose(broken,0);
  require(broken.page==App::Message&&broken.messageText().find("drei")!=std::string::npos,"UI wrong-package error");broken.key(Back);require(broken.page==App::Home,"wrong package returns home");
  choose(broken,App::AlphabetSearch);require(broken.page==App::Message,"alphabet rejects wrong package");broken.key(Back);
  choose(broken,App::CalculatorMenu);choose(broken,0);choose(broken,0);broken.key('2');broken.key(Enter);require(broken.messageText().find("T = 0.5 s")!=std::string::npos,"calculator survives navigation failure");
  std::cout<<"PASS NAVIGATION: "<<checks<<" checks; "<<nodes.size()<<" nodes; 546 chapters; 178 theory; 400 formulas per search; "
           <<linkCounts[0]+linkCounts[1]+linkCounts[2]+linkCounts[3]<<" links; "<<linkCounts[0]+linkCounts[2]+linkCounts[3]<<" public links; 142 alphabet quantities; typed refs, folder placement, corruption, back, touch, 20 images\n";
}

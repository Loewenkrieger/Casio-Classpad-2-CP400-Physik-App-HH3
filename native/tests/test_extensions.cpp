// SPDX-License-Identifier: GPL-3.0-or-later
#include "app.hpp"
#include "knowledge.hpp"
#include "browse_helpers.hpp"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <limits>
#include <set>

namespace {
int checks=0;
void require(bool ok,const std::string& why){++checks;if(!ok){std::cerr<<"FAIL EXTENSIONS: "<<why<<'\n';std::exit(1);}}
std::string read(const std::string& path){std::ifstream f(path,std::ios::binary);require(f.good(),"read "+path);return {std::istreambuf_iterator<char>(f),{}};}
void write(const std::string& path,const std::string& data){std::ofstream f(path,std::ios::binary);f.write(data.data(),data.size());require(f.good(),"write "+path);}
void choose(physik::App& app,const std::string& title){
  const auto items=app.labels();const auto found=std::find(items.begin(),items.end(),title);require(found!=items.end(),"find menu "+title);
  const int target=found-items.begin();while(app.selected<target)app.key(physik::Down);while(app.selected>target)app.key(physik::Up);app.key(physik::Enter);
}
void tap(physik::App& app,int x,int y){app.touchDown(x,y);app.touchUp();}
void home(physik::App& app){for(int i=0;i<10&&app.page!=physik::App::Home;++i)app.key(physik::Back);require(app.page==physik::App::Home&&app.running,"home");}
void snapshot(const std::string& name,const std::vector<uint16_t>& pixels){
  std::ofstream out("build/preview/"+name+".ppm",std::ios::binary);out<<"P6\n320 528\n255\n";
  for(uint16_t p:pixels){out.put(((p>>11)&31)*255/31);out.put(((p>>5)&63)*255/63);out.put((p&31)*255/31);}require(out.good(),"preview");
}
std::vector<uint16_t> render(physik::App& app){
  std::vector<uint16_t> pixels(320*528);physik::Canvas canvas(pixels.data());app.draw(canvas);return pixels;
}
void axes(physik::App& app,std::initializer_list<const char*> values){
  using namespace physik;
  app.key('A');require(app.page==App::Form&&values.size()==4,"open axes editor");
  for(const char* text:values){app.key(127);for(;*text;++text)app.key(*text);app.key(Enter);}
  require(app.page==App::Chart,"accept manual bounds");
}
void plotRegressions(){
  using namespace physik;
  const double huge=std::numeric_limits<double>::max();
  const double tiny=std::numeric_limits<double>::denorm_min();
  auto clipped=[&](std::pair<double,double> a,std::pair<double,double> z,
                   std::pair<double,double> ca,std::pair<double,double> cz,const PlotBounds& b){
    Plot actual{"clipping","x","m","y","m",{a,z},{},true},expected=actual;
    expected.points={ca,cz};
    std::vector<uint16_t> pixels(320*528+2,0x1357),reference(pixels.size(),0x1357);
    Canvas canvas(pixels.data()+1),other(reference.data()+1);
    require(validPlotBounds(b),"finite clipping bounds");
    drawPlot(canvas,actual,b,actual.points.size());drawPlot(other,expected,b,expected.points.size());
    require(pixels.front()==0x1357&&pixels.back()==0x1357,"extreme clipping stays in framebuffer");
    bool equal=true,visible=false;
    for(int y=100;y<362;++y)for(int x=65;x<301;++x){
      const size_t i=1+y*320+x;
      equal=equal&&pixels[i]==reference[i];visible=visible||pixels[i]==0x2490;
    }
    require(equal&&visible,"extreme segment matches explicitly clipped reference");
  };
  clipped({-1e308,0},{1e308,1},{-.5,.5},{.5,.5},{-.5,.5,0,1});
  clipped({-huge,-huge},{huge,huge},{-.5,-.5},{.5,.5},{-.5,.5,-.5,.5});
  clipped({huge,huge},{-huge,-huge},{.5,.5},{-.5,-.5},{-.5,.5,-.5,.5});
  clipped({0,-huge},{1,huge},{.5,-.5},{.5,.5},{0,1,-.5,.5});
  clipped({-huge,-huge},{huge,huge},{tiny,tiny},{4*tiny,4*tiny},{tiny,4*tiny,tiny,4*tiny});
  clipped({-huge,-huge},{huge,huge},{-huge,-huge},{huge,huge},{-huge,huge,-huge,huge});
  const double a=1.0000000000000004e-305,z=std::nextafter(a,huge);
  clipped({a,a},{z,z},{a,a},{z,z},{a,z,a,z});
  Plot p{"touch","x","m","y","m",{{-10,0},{0,1},{10,2}},{},false};
  const PlotBounds b{0,10,0,2};
  require(plotCursor(p,b,60)==1,"hidden x point cannot intercept visible left endpoint");
  require(plotCursor(p,b,305)==2,"visible right endpoint remains selectable");
  p.points={{0,-10},{0,1},{10,20}};
  require(plotCursor(p,b,60)==1&&plotCursor(p,b,305)==1,"touch ignores hidden y points on both edges");
  require(plotCursor(p,b,std::numeric_limits<int>::min())==1&&
          plotCursor(p,b,std::numeric_limits<int>::max())==1,"touch pixel distance cannot overflow");
  p.points={{-10,0},{0,-10},{10,20}};
  require(plotCursor(p,b,60)==p.points.size(),"no visible point returns no hit");
  require(plotCursor(p,{0,0,0,1},60)==p.points.size(),"invalid bounds return no hit");
  p.points.clear();require(plotCursor(p,b,60)==0,"empty touch series");
}
void plotStateRegressions(){
  using namespace physik;
  App app("build/package/");
  Workflow chosen;std::string categoryTitle;bool found=false;
  for(const auto& category:simulationCategories()){
    for(const auto& w:simulationCatalog(category.id)){
      if(w.maxCount||w.defaults.size()!=w.calculation.parameters.size())continue;
      std::vector<double> values;std::string error;bool complete=true;
      for(size_t i=0;i<w.defaults.size();++i){
        const auto options=nativeUnits(w.calculation.parameters[i]);
        const auto unit=std::find_if(options.begin(),options.end(),[&](const Unit& u){return u.name==w.calculation.parameters[i].defaultUnit;});
        double x=0;if(unit==options.end()||!parseNumber(w.defaults[i],unit->scale,x,error)){complete=false;break;}
        values.push_back(x);
      }
      NativeResult result;
      if(complete&&nativeCalculate(w,values,result,error)&&result.plots.size()>=2){chosen=w;categoryTitle=category.title;found=true;break;}
    }
    if(found)break;
  }
  require(found,"two-plot workflow for axes persistence");
  choose(app,"Physik-Rechner");choose(app,categoryTitle);choose(app,chosen.calculation.title);
  bool alternateUnit=false;
  for(size_t i=0;i<chosen.calculation.parameters.size();++i){
    const auto& parameter=chosen.calculation.parameters[i];const auto options=nativeUnits(parameter);
    if(!alternateUnit&&options.size()>1){
      const auto unit=std::find_if(options.begin(),options.end(),[&](const Unit& u){return u.name==parameter.defaultUnit;});
      require(unit!=options.end(),"unit backup setup has default unit");
      const auto next=(static_cast<size_t>(unit-options.begin())+1)%options.size();
      double si=0;std::string error;
      require(parseNumber(chosen.defaults[i],unit->scale,si,error),"unit backup setup parses default");
      app.key('U');app.key(127);
      for(char c:formatNumber(si/options[next].scale,15))app.key(c);
      alternateUnit=true;
    }
    app.key(Enter);
  }
  require(alternateUnit,"axes backup uses a nondefault input unit");
  require(app.page==App::Message,"graph state setup computes defaults");
  app.key(Back);const auto originalForm=render(app);const int originalField=app.field;
  for(int i=0;i<originalField;++i)app.key(Left);
  const auto originalFirstForm=render(app);
  app.key('=');app.key(Enter);
  require(app.page==App::Chart,"graph state setup enters first plot");
  const auto autoFirst=render(app);
  axes(app,{"0","0.5","0","2"});const auto manualFirst=render(app);
  require(manualFirst!=autoFirst,"manual first bounds differ from automatic bounds");
  app.key(Back);app.key(Enter);
  require(render(app)==manualFirst,"manual bounds survive Back and reopen");
  app.key(Enter);require(app.page==App::TableView,"open table with manual bounds");
  app.key(Down);app.key(Enter);
  require(render(app)==manualFirst,"table EXE preserves manual bounds");
  app.key(Enter);app.key(Back);app.key(Enter);
  require(render(app)==manualFirst,"table Back and reopen preserves manual bounds");
  app.key(Down);const auto autoSecond=render(app);
  axes(app,{"-2","3","-4","5"});const auto manualSecond=render(app);
  require(manualSecond!=autoSecond,"second plot has distinct manual bounds");
  app.key(Up);require(render(app)==manualFirst,"first plot retains its own bounds");
  app.key(Down);require(render(app)==manualSecond,"second plot retains its own bounds");
  axes(app,{"1000","1001","1000","1001"});const auto emptyView=render(app);
  tap(app,60,200);require(render(app)==emptyView,"touch without visible points preserves selection");
  axes(app,{"-2","3","-4","5"});
  app.key('A');app.key('9');app.key(Back);
  require(render(app)==manualSecond,"cancel axes preserves committed bounds");
  app.key('S');require(render(app)==autoSecond,"explicit autoscale resets only current plot");
  axes(app,{"-2","3","-4","5"});app.key('A');tap(app,277,450);
  require(app.page==App::Chart&&render(app)==autoSecond,"axes Auto button restores form and autoscales");
  app.key(Up);require(render(app)==manualFirst,"autoscaling another plot preserves first bounds");
  app.key(Back);app.key(Back);
  require(app.page==App::Form&&render(app)==originalFirstForm,"axes backup preserves first field's value and nondefault unit");
  for(int i=0;i<originalField;++i)app.key(Enter);
  require(render(app)==originalForm,"axes backup preserves remaining values and units");
  app.key('=');require(app.page==App::Message,"recompute after axes backup");app.key(Enter);
  require(render(app)==autoFirst,"new computation discards old manual bounds");
  app.key(Down);require(render(app)==autoSecond,"new computation resets every plot's bounds");
}
size_t articleDamageRegressions(const std::string& directory){
  using namespace physik;
  Store package("build/package/");std::string text,error;std::vector<std::string> media;
  const auto base=read("../wissen.txt"),combined=read("build/package/wissen.txt");
  require(combined.size()>base.size()&&combined.compare(0,base.size(),base)==0,"frozen prefix exact");
  const auto lastArticle=static_cast<uint32_t>(supplementCount()-1);std::string lastArticleText;
  require(supplementLoad(package,lastArticle,lastArticleText,media,error),error);
  const auto articlePosition=combined.find(lastArticleText);
  require(!lastArticleText.empty()&&articlePosition!=std::string::npos,"locate final article before appended catalog");
  const auto articleEnd=articlePosition+lastArticleText.size()-1;
  std::filesystem::create_directories(directory);
  auto bad=combined;bad[base.size()+8]^=1;write(directory+"wissen.txt",bad);Store broken(directory);
  require(!supplementLoad(broken,0,text,media,error)&&broken.chapter(0,text),"bad extension marker leaves base usable");
  bad=combined;bad[articleEnd]^=1;write(directory+"wissen.txt",bad);
  require(!supplementLoad(broken,lastArticle,text,media,error)&&text.empty(),"article checksum");
  write(directory+"wissen.txt",combined.substr(0,articleEnd));
  require(!supplementLoad(broken,lastArticle,text,media,error),"truncated supplement");
  return combined.size()-articleEnd-1;
}
}
int main(int argc,char** argv){
  using namespace physik;
  if(argc==2&&std::string(argv[1])=="--plots-only"){
    plotRegressions();plotStateRegressions();
    std::cout<<"PASS PLOTS: "<<checks<<" checks; extreme clipping, visible touch, per-plot bounds/back/table/recompute/auto\n";
    return 0;
  }
  if(argc==2&&std::string(argv[1])=="--article-only"){
    const auto suffix=articleDamageRegressions("build/plot-agent-bad/");
    std::cout<<"PASS ARTICLE DAMAGE: "<<checks<<" checks; final article targeted before "<<suffix<<" trailing bytes\n";
    return 0;
  }
  std::filesystem::create_directories("build/preview");
  std::set<std::string> categoryIds,workflowIds;
  const auto categories=nativeCategories();require(categories.size()>=5,"all modules registered");size_t workflows=0;
  std::ofstream inventory("build/native-catalog.json");inventory<<"[\n";bool comma=false;
  App app("build/package/");std::vector<uint16_t> frame(320*528);Canvas canvas(frame.data());
  for(const auto& category:categories){
    require(categoryIds.insert(category.id).second,"unique category "+category.id);
    const auto catalog=nativeCatalog(category.id);require(!catalog.empty(),"nonempty category");
    home(app);choose(app,"Physik-Rechner");choose(app,category.title);
    require(app.page==App::Calcs&&app.labels().size()==catalog.size(),"all native menu entries");
    for(const auto& w:catalog){
      const auto& calc=w.calculation;++workflows;
      require(workflowIds.insert(calc.id).second,"unique workflow "+calc.id);
      require(!calc.title.empty()&&!calc.formula.empty()&&!calc.parameters.empty(),"workflow metadata "+calc.id);
      require(w.defaults.size()<=calc.parameters.size(),"base default count "+calc.id);
      if(w.maxCount)require(w.minCount>0&&w.minCount<=w.maxCount&&w.maxCount<=40&&!w.repeated.empty(),"bounded repeat "+calc.id);
      Workflow copy;require(nativeWorkflow(calc.id,copy)&&copy.calculation.title==calc.title,"lookup "+calc.id);
      std::vector<Parameter> params;std::vector<std::string> defaults;expandWorkflow(w,w.minCount,params,defaults);
      require(params.size()==calc.parameters.size()+w.minCount*w.repeated.size()&&defaults.size()==params.size(),"expansion "+calc.id);
      for(const auto& p:params){
        const auto options=nativeUnits(p);require(!options.empty(),"units "+calc.id);
        require(std::any_of(options.begin(),options.end(),[&](const Unit& u){return u.name==p.defaultUnit;}),"default unit "+calc.id+" "+p.name+" "+p.defaultUnit);
        for(const auto& u:options)require(std::isfinite(u.scale)&&u.scale>0,"positive unit scale");
      }
      NativeResult result;std::string error;
      require(!nativeCalculate(w,{},result,error)&&!error.empty()&&result.values.empty()&&result.plots.empty(),"missing inputs "+calc.id);
      std::vector<double> invalid(params.size(),0);if(w.maxCount)invalid[0]=w.minCount;
      invalid.back()=std::numeric_limits<double>::quiet_NaN();
      require(!nativeCalculate(w,invalid,result,error)&&!error.empty(),"NaN "+calc.id);
      choose(app,calc.title);require(app.page==App::Form,"open every workflow "+calc.id);app.draw(canvas);app.key(Back);
      require(app.page==App::Calcs,"return from every workflow");
      if(comma)inventory<<",\n";
      comma=true;
      inventory<<"  {\"category\":"<<std::quoted(category.title)<<",\"id\":"<<std::quoted(calc.id)<<",\"title\":"<<std::quoted(calc.title)<<",\"inputs\":"<<calc.parameters.size()<<",\"min_count\":"<<w.minCount<<",\"max_count\":"<<w.maxCount<<"}";
    }
  }
  inventory<<"\n]\n";inventory.close();require(inventory.good(),"catalog inventory");
  require(nativeCatalog("unknown").empty(),"unknown category");Workflow absent;require(!nativeWorkflow("unknown",absent),"unknown workflow");
  require(supplementCount()==32,"native article count");Store package("build/package/"),old("../");
  std::string text,error;std::vector<std::string> media;
  for(uint32_t i=0;i<supplementCount();++i){require(supplementLoad(package,i,text,media,error),error);require(text.size()>200,"substantive article");
    for(const auto& id:media){Media image;require(package.media(id,image),"existing article media "+id);}
  }
  require(!supplementLoad(package,supplementCount(),text,media,error)&&text.empty()&&media.empty(),"article bounds");
  require(!supplementLoad(old,0,text,media,error)&&error.find("drei")!=std::string::npos,"old data mismatch explains replacement");
  require(old.chapter(0,text),"legacy article remains readable");
  articleDamageRegressions("build/supplement_bad/");
  size_t links=0;
  for(int mode=0;mode<3;++mode)for(const auto& section:supplementSections(mode)){
    const auto entries=supplementEntries(section.id);require(!entries.empty(),"supplement section");links+=entries.size();
  }
  require(links==supplementLinkCount(),"all supplemental links counted");
  const unsigned visibleModes[]={0,2,3};
  for(unsigned mode=0;mode<3;++mode)for(uint32_t id=0;id<supplementCount();++id){
    std::vector<int> path;require(browse_test::find(app.store,navigationRoot(visibleModes[mode]),NavigationKind::NativeArticle,id,path,error),"native article in new hierarchy: "+error);
    home(app);choose(app,App::homeItems[mode]);
    for(int index:path){const auto labels=app.labels();require(index>=0&&index<static_cast<int>(labels.size()),"path index");choose(app,labels[index]);}
    require(app.page==App::Article,"new article via all searches");app.key(Back);
  }
  Plot p{"Signed points","t","s","v","m/s",{{0,-2},{1,0},{2,2}},{{0,-1},{2,1}},true};PlotBounds b;
  require(plotBounds(p,b,error)&&validPlotBounds(b),"signed bounds");
  require(plotCursor(p,b,60)==0&&plotCursor(p,b,305)==2,"touch cursor");
  std::vector<uint16_t> guarded(320*528+2,0x1357);Canvas guard(guarded.data()+1);drawPlot(guard,p,b,1000);
  require(guarded.front()==0x1357&&guarded.back()==0x1357,"plot clipping memory");
  p.points={{0,0}};p.model.clear();require(plotBounds(p,b,error)&&validPlotBounds(b),"single-point axes");
  p.points.clear();require(!plotBounds(p,b,error),"empty plot");p.points={{1,std::numeric_limits<double>::infinity()}};require(!plotBounds(p,b,error),"nonfinite plot");
  require(!validPlotBounds({0,0,0,1})&&!validPlotBounds({0,1,2,1}),"invalid manual bounds");
  NativeResult poisoned;poisoned.values.push_back({"bad",std::numeric_limits<double>::infinity(),"1"});require(!nativeFinish(poisoned,error)&&poisoned.values.empty(),"finite output guard");
  plotRegressions();plotStateRegressions();

  // Use a real model's defaults, then exercise the shared graph interface.
  bool graphChecked=false;
  for(const auto& category:simulationCategories())for(const auto& w:simulationCatalog(category.id)){
    if(graphChecked||w.maxCount||w.defaults.size()!=w.calculation.parameters.size())continue;
    std::vector<double> values;bool complete=true;
    for(size_t i=0;i<w.defaults.size();++i){const auto options=nativeUnits(w.calculation.parameters[i]);
      auto u=std::find_if(options.begin(),options.end(),[&](const Unit& a){return a.name==w.calculation.parameters[i].defaultUnit;});double x=0;
      if(u==options.end()||!parseNumber(w.defaults[i],u->scale,x,error)){complete=false;break;}values.push_back(x);
    }
    NativeResult r;if(!complete||!nativeCalculate(w,values,r,error)||r.plots.empty()||r.values.empty())continue;
    home(app);choose(app,"Physik-Rechner");choose(app,category.title);choose(app,w.calculation.title);
    for(size_t i=0;i<values.size();++i)app.key(Enter);
    require(app.page==App::Message&&app.messageText().find(r.values[0].name)!=std::string::npos,"model defaults via actual form");
    app.draw(canvas);snapshot("new_model_result",frame);app.key(Enter);require(app.page==App::Chart,"EXE opens plot");app.draw(canvas);snapshot("new_model_plot",frame);
    app.key(Right);app.key(Left);tap(app,200,220);app.draw(canvas);
    app.key(Enter);require(app.page==App::TableView,"table via EXE");app.draw(canvas);snapshot("new_model_table",frame);
    app.touchDown(100,300);app.touchMove(100,180);app.touchUp();app.key(Enter);require(app.page==App::Chart,"return from table");
    tap(app,270,505);require(app.page==App::Form,"manual axes");app.draw(canvas);snapshot("new_axes_form",frame);
    for(int i=0;i<4;++i){app.key('0');app.key(Enter);}require(app.page==App::Form,"equal bounds rejected");
    app.key(Back);require(app.page==App::Chart,"cancel axes restores graph");
    tap(app,270,505);for(int i=0;i<4;++i)app.key(Enter);require(app.page==App::Chart,"accept manual axes");
    for(int i=0;i<20;++i){tap(app,150,505);app.draw(canvas);require(app.page==App::Chart,"twenty plot switches");}
    app.key(Back);app.key(Back);require(app.page==App::Form,"graph return preserves original form");app.key('=');require(app.page==App::Message,"recompute after axes editor");
    graphChecked=true;
  }
  require(graphChecked,"real simulation plot tested");home(app);choose(app,"Beenden");require(!app.running,"exit after extensions");
  std::cout<<"PASS EXTENSIONS: "<<checks<<" checks; "<<categories.size()<<" categories; "<<workflows<<" workflows; "<<supplementCount()<<" articles; "<<links<<" links\n";
}

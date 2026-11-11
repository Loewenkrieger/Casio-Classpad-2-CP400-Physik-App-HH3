// SPDX-License-Identifier: GPL-3.0-or-later
#include "app.hpp"
#include <algorithm>
#include <cmath>
#include <cstdlib>

namespace physik {
namespace {
constexpr uint16_t bg=0xef9d,fg=0x1144,accent=0x2490,selectedColor=0xb6b6,white=0xffff,dim=0x6b6d,red=0xa800;
const char* keypad[4][4]={{"7","8","9","DEL"},{"4","5","6","-"},{"1","2","3","E"},{"0",".","OK","Einheit"}};
bool integer(const Value& value,uint32_t& result){
  if(value.type!=Value::Integer||value.number<0||value.number>UINT32_MAX)return false;
  result=static_cast<uint32_t>(value.number);return true;
}
bool triple(const Value& value,uint32_t& start,uint32_t& count){
  return value.type==Value::Tuple&&value.items.size()==3&&value.items[0].type==Value::Text&&integer(value.items[1],start)&&integer(value.items[2],count);
}
void header(Canvas& c,const std::string& title,bool back){
  c.rect(0,0,320,42,accent);if(back)c.text(8,10,"<",white);
  c.text(back?38:12,10,c.fit(title,back?274:298),white);
}
void footer(Canvas& c,const char* a,const char* b,const char* last){
  c.rect(0,482,320,46,selectedColor);c.text(16,495,a,fg);c.text(124,495,b,fg);c.text(221,495,last,fg);
}
bool menuPage(App::Page page){return page==App::Theory||page==App::Entries||page==App::CalcCategories||page==App::Calcs;}
}
std::vector<std::string> App::labels()const{
  if(page==Home)return {homeItems,homeItems+5};
  std::vector<std::string> result;
  if(menuPage(page)&&!menus.empty())for(const auto& row:menus.back().rows)result.push_back(row.items[0].text);
  return result;
}
void App::notify(std::string text,Page destination,const std::string& heading){
  message=std::move(text);messageBack=destination;messageTitle=heading;
  uint16_t pixel=0;Canvas measure(&pixel);messageLines=measure.wrap(message,304);messageScroll=0;page=Message;dirty=true;
}
void App::ensureVisible(){
  const int count=page==Home?5:menus.empty()?0:static_cast<int>(menus.back().rows.size());
  if(count==0){selected=first=0;return;}
  selected=std::clamp(selected,0,count-1);
  if(selected<first)first=selected;
  if(selected>=first+8)first=selected-7;
  first=std::clamp(first,0,std::max(0,count-8));
}
bool App::openMenu(Page kind,const std::string& heading,Table table,uint32_t start,uint32_t count){
  std::vector<Value> rows;
  if(!store.rows(table,start,count,rows)){notify(store.error(),page);return false;}
  for(const auto& row:rows)if(row.type!=Value::Tuple||row.items.empty()||row.items[0].type!=Value::Text){notify("Ungueltiger Menueeintrag",page);return false;}
  if(rows.empty()){notify("Keine Eintraege",page);return false;}
  if(!menus.empty()){menus.back().selected=selected;menus.back().first=first;}
  menus.push_back({kind,heading,std::move(rows),0,0});selected=first=0;page=kind;dirty=true;return true;
}
void App::openArticle(const Value& value){
  uint32_t chapter=0;
  if(value.type!=Value::Tuple||value.items.size()!=2||value.items[0].type!=Value::Text||!integer(value.items[1],chapter)){
    notify("Ungueltiger Artikel",Entries);return;
  }
  std::string source;
  if(!store.chapter(chapter,source)||!store.images(chapter,ids)){notify(store.error(),Entries);return;}
  title=value.items[0].text;
  uint16_t pixel=0;Canvas measure(&pixel);lines=measure.wrap(source,304);scroll=0;page=Article;dirty=true;
}
void App::activate(){
  if(page==Home){
    homeSelection=selected;
    if(selected==4){running=false;return;}
    if(selected==3){openMenu(CalcCategories,"Physik-Rechner",calcCategories,0,calcCategories.count);return;}
    Value mode;uint32_t start=0,count=0;
    if(!store.row(modes,static_cast<uint32_t>(selected),mode)){notify(store.error(),Home);return;}
    if(!triple(mode,start,count)){notify("Ungueltiger Themenbereich",Home);return;}
    openMenu(Theory,mode.items[0].text,sections,start,count);return;
  }
  if(menuPage(page)&&!menus.empty()){
    const auto& row=menus.back().rows[static_cast<size_t>(selected)];
    if(page==Theory||page==CalcCategories){
      uint32_t start=0,count=0;
      if(!triple(row,start,count)){notify("Ungueltiger Themenbereich",page);return;}
      openMenu(page==Theory?Entries:Calcs,row.items[0].text,page==Theory?entries:calculations,start,count);
    }else if(page==Entries)openArticle(row);
    else beginCalculation(row);
  }else if(page==Article&&!ids.empty()){image=0;page=Image;}
  else if(page==Form)nextField(1);
  else if(page==Image||page==Message)back();
  dirty=true;
}
void App::back(){
  if(page==Home){running=false;return;}
  if(page==Message){page=messageBack;}
  else if(page==Image)page=Article;
  else if(page==Article)page=Entries;
  else if(page==Form)page=Calcs;
  else if(menuPage(page)){
    menus.pop_back();
    if(menus.empty()){page=Home;selected=homeSelection;first=0;}
    else {page=menus.back().page;selected=menus.back().selected;first=menus.back().first;}
  }
  dirty=true;
}
void App::moveScroll(int delta){
  if(page==Message)messageScroll=std::clamp(messageScroll+delta,0,std::max(0,static_cast<int>(messageLines.size())-20));
  else scroll=std::clamp(scroll+delta,0,std::max(0,static_cast<int>(lines.size())-20));
  dirty=true;
}
void App::beginCalculation(const Value& value){
  if(!calculationRecord(value,calculation,error)){notify(error,Calcs);return;}
  fitting=calculation.id.rfind("FIT_",0)==0;
  if(fitting&&calculation.id!="FIT_F2"&&calculation.id!="FIT_FN"&&calculation.id!="FIT_L2"&&calculation.id!="FIT_LN"){
    notify("Unbekannte Messreihe",Calcs);return;
  }
  wavelength=fitting&&calculation.id.rfind("FIT_L",0)==0;
  variableCount=fitting&&calculation.id.back()=='N';
  pointCount=0;parameters=calculation.parameters;
  if(fitting)fitParameters(variableCount?0:2);
  if(parameters.empty()){notify("Rechner ohne Eingabeparameter",Calcs);return;}
  inputs.assign(parameters.size(),"");units.assign(parameters.size(),0);
  for(size_t i=0;i<parameters.size();++i){const auto options=unitOptions(parameters[i].unit);
    for(size_t j=0;j<options.size();++j)if(options[j].name==parameters[i].defaultUnit)units[i]=j;
  }
  field=0;error.clear();replaceOnType=false;page=Form;dirty=true;
}
void App::fitParameters(int count){
  parameters.clear();pointCount=count;
  if(variableCount)parameters.push_back({"Messpunkte (3-10)","1","1"});
  for(int i=0;i<count;++i){
    const std::string point="Punkt "+std::to_string(i+1)+"/"+std::to_string(count)+": ";
    parameters.push_back({point+(wavelength?"lambda":"f"),wavelength?"m":"Hz",wavelength?"nm":"THz"});
    parameters.push_back({point+(wavelength?"E_kin":"U_G"),wavelength?"J":"V",wavelength?"eV":"V"});
  }
}
bool App::validateField(double& value){
  if(field<0||field>=static_cast<int>(parameters.size())){error="Ungueltiges Eingabefeld";return false;}
  const auto options=unitOptions(parameters[field].unit);
  return parseNumber(inputs[field],options[units[field]].scale,value,error);
}
void App::nextField(int direction){
  if(direction<0){field=std::max(0,field-1);replaceOnType=!inputs[field].empty();error.clear();dirty=true;return;}
  double value=0;if(!validateField(value)){dirty=true;return;}
  if(fitting&&variableCount&&field==0){
    if(value!=std::floor(value)||value<3||value>10){error="Bitte 3 bis 10 ganze Messpunkte.";dirty=true;return;}
    if(pointCount!=static_cast<int>(value)){
      const std::string countText=inputs[0];fitParameters(static_cast<int>(value));
      inputs.assign(parameters.size(),"");units.assign(parameters.size(),0);inputs[0]=countText;
      for(size_t i=1;i<parameters.size();++i){const auto options=unitOptions(parameters[i].unit);
        for(size_t j=0;j<options.size();++j)if(options[j].name==parameters[i].defaultUnit)units[i]=j;}
    }
  }
  if(direction>0&&field==static_cast<int>(parameters.size())-1){compute();return;}
  field=std::clamp(field+direction,0,static_cast<int>(parameters.size())-1);replaceOnType=!inputs[field].empty();dirty=true;
}
void App::compute(){
  if(fitting&&variableCount&&field==0&&pointCount==0){nextField(1);return;}
  std::vector<double> values;const int previousField=field;
  for(size_t i=0;i<parameters.size();++i){field=static_cast<int>(i);double value=0;if(!validateField(value)){dirty=true;return;}values.push_back(value);}
  field=previousField;std::string body=calculation.formula+"\n\n";
  if(fitting){
    const size_t start=variableCount?1:0;std::vector<std::pair<double,double>> points;
    if(variableCount&&(values[0]!=pointCount)){field=0;error="Messpunktzahl erneut bestaetigen";dirty=true;return;}
    for(size_t i=start;i+1<values.size();i+=2)points.emplace_back(values[i],values[i+1]);
    Fit fit;
    if(!fitPlanck(points,wavelength,fit,error)){replaceOnType=true;notify(error,Form,"Eingabe pruefen");return;}
    body+=resultText("h_fit",fit.h,"Js")+"\n\n"+resultText("W_A",fit.work,"J")+"\n\n"+resultText("f_G",fit.threshold,"Hz");
    body+="\n\nRMS="+formatNumber(fit.rms)+(wavelength?" eV":" V");
    body+="\nUngewichteter Fit; keine Fehlerabschaetzung. Konstanten bleiben unveraendert.";
  }else {
    double result=0;
    if(!calculate(calculation,values,result,error)){replaceOnType=true;notify(error,Form,"Eingabe pruefen");return;}
    body+=resultText(calculation.name,result,calculation.unit);
    std::string note;
    if(store.note(calculation.id,note)){if(!note.empty())body+="\n\nGeltung:\n"+note;}
    else body+="\n\nGeltungshinweis nicht lesbar:\n"+store.error();
  }
  replaceOnType=true;notify(body,Form,calculation.title);
}
void App::edit(char key){
  if(std::string("0123456789.,-+eE").find(key)==std::string::npos)return;
  std::string value=replaceOnType?"":inputs[field];
  if(value.size()>=96){error="Maximal 96 Zeichen";dirty=true;return;}
  if(key==',')key='.';
  if(key=='e')key='E';
  if(key=='.'){
    if(value.find('.')!=std::string::npos||value.find('E')!=std::string::npos)return;
    if(value.empty()||value=="-"||value=="+")value+='0';
  }else if(key=='E'){
    if(value.find('E')!=std::string::npos||value.find_first_of("0123456789")==std::string::npos)return;
  }else if(key=='-'||key=='+'){
    if(!value.empty()&&value.back()!='E')return;
  }
  value+=key;inputs[field]=std::move(value);replaceOnType=false;error.clear();dirty=true;
}
void App::changeUnit(){
  const auto options=unitOptions(parameters[field].unit);units[field]=(units[field]+1)%options.size();dirty=true;
}
void App::key(int code,bool repeat){
  if(!running||(repeat&&(code==Enter||code==Back||code==Clear||code=='=')))return;
  if(code==Back||code==Clear){back();return;}
  if(code==127){if(page==Form){if(replaceOnType){inputs[field].clear();replaceOnType=false;}else if(!inputs[field].empty())inputs[field].pop_back();error.clear();dirty=true;}else back();return;}
  if(code==Enter){activate();return;}
  if(page==Form){
    if(code=='='){compute();return;}
    if(code==Left||code==Up){nextField(-1);return;}
    if(code==Right||code==Down){nextField(1);return;}
    if(code=='U'){changeUnit();return;}
    if(code>=32&&code<127)edit(static_cast<char>(code));
    return;
  }
  if(page==Article||page==Message){moveScroll(code==Up?-1:code==Down?1:code==Left?-19:code==Right?19:0);return;}
  if(page==Image&&(code==Left||code==Right)){
    if(!ids.empty())image=(image+(code==Left?-1:1)+static_cast<int>(ids.size()))%static_cast<int>(ids.size());
    dirty=true;return;
  }
  if(page==Home||menuPage(page)){selected+=code==Up?-1:code==Down?1:code==Left?-8:code==Right?8:0;ensureVisible();dirty=true;}
}
void App::touchDown(int x,int y){
  if(x<0||x>=320||y<0||y>=528)return;
  touchX=x;touchY=y;startScroll=page==Message?messageScroll:page==Article?scroll:first;dragged=cancelled=false;
}
void App::touchMove(int x,int y){
  if(touchX<0)return;
  if(x<0||x>=320||y<0||y>=528){cancelled=true;return;}
  if((page==Article||page==Message)&&touchY>=52&&touchY<482&&std::abs(y-touchY)>10){
    dragged=true;if(page==Message)messageScroll=startScroll;else scroll=startScroll;moveScroll((touchY-y)/20);
  }else if(menuPage(page)&&touchY>=58&&touchY<482&&std::abs(y-touchY)>12){
    dragged=true;const int count=static_cast<int>(menus.back().rows.size());first=std::clamp(startScroll+(touchY-y)/48,0,std::max(0,count-8));
    selected=std::clamp(selected,first,std::min(first+7,count-1));dirty=true;
  }else if(std::abs(x-touchX)>12||std::abs(y-touchY)>12)cancelled=true;
}
void App::touchUp(){if(touchX>=0&&!cancelled&&!dragged)tap(touchX,touchY);touchX=touchY=-1;}
void App::tap(int x,int y){
  if(y<42&&x<42&&page!=Home){back();return;}
  if(page==Home||menuPage(page)){
    const int index=first+(y-58)/48,count=page==Home?5:static_cast<int>(menus.back().rows.size());
    if(y>=58&&y<442&&index<count){selected=index;activate();}return;
  }
  if(page==Form){
    if(y>=482){if(x<106)back();else if(x<213)nextField(1);else compute();return;}
    if(y>=250&&y<476&&x>=7&&x<313){
      const int row=(y-250)/58,col=(x-7)/78;
      if(row<4&&col<4&&(y-250)%58<52&&(x-7)%78<72){
        const std::string key=keypad[row][col];
        if(key=="DEL")this->key(127);else if(key=="OK")nextField(1);else if(key=="Einheit")changeUnit();else edit(key[0]);
      }
    }return;
  }
  if(y<482)return;
  if(page==Article){if(x<105)moveScroll(-19);else if(x<210)moveScroll(19);else if(ids.empty())back();else activate();}
  else if(page==Image){if(x<105)key(Left);else if(x<210)key(Right);else back();}
  else if(page==Message){if(x<105)moveScroll(-19);else if(x<210)moveScroll(19);else back();}
}
void App::draw(Canvas& c){
  c.rect(0,0,320,528,bg);
  if(page==Home||menuPage(page)){
    header(c,page==Home?"Physik LK Native":menus.back().title,page!=Home);
    const auto items=labels();
    for(int slot=0;slot<8&&first+slot<static_cast<int>(items.size());++slot){
      const int index=first+slot,y=58+slot*48;if(index==selected)c.rect(6,y-3,308,44,selectedColor);
      c.text(12,y+8,c.fit(items[index],296),fg);
    }
    if(page==Home){c.text(10,395,"Native Beta 2.0.0-beta.1",dim);c.text(10,420,"Theorie, Formeln und Rechner",dim,true);}
    c.text(10,495,"EXE: oeffnen   Clear: zurueck",dim,true);
  }else if(page==Article){
    header(c,title,true);
    for(int i=0;i<20&&scroll+i<static_cast<int>(lines.size());++i)c.text(8,56+i*20,lines[scroll+i],fg);
    footer(c,"<",">",ids.empty()?"Zurueck":"Bilder");
  }else if(page==Image){
    Media media;std::vector<uint8_t> data;std::string failure;
    if(image<0||image>=static_cast<int>(ids.size())||!store.media(ids[image],media)||!store.imageData(media,data)){
      notify(store.error()+"\nTheorie bleibt nutzbar.",Article);draw(c);return;
    }
    c.rect(0,0,320,528,white);header(c,media.title,true);
    c.text(142,45,std::to_string(image+1)+" / "+std::to_string(ids.size()),fg,true);
    if(!drawMedia(c,media,data,failure)){notify(failure+"\nTheorie bleibt nutzbar.",Article);draw(c);return;}
    const auto captions=c.wrap(media.caption,304);
    for(size_t i=0;i<std::min<size_t>(4,captions.size());++i)c.text(8,400+static_cast<int>(i)*20,captions[i],fg);
    footer(c,"<",">","Zurueck");
  }else if(page==Form){
    header(c,calculation.title,true);const auto formula=c.wrap(calculation.formula,304,true);
    for(size_t i=0;i<std::min<size_t>(4,formula.size());++i)c.text(8,54+static_cast<int>(i)*16,formula[i],fg,true);
    const auto options=unitOptions(parameters[field].unit);
    const std::string label=std::to_string(field+1)+"/"+std::to_string(parameters.size())+" "+parameters[field].name+" ["+options[units[field]].name+"]";
    c.text(8,142,c.fit(label,304),fg);
    c.rect(8,174,304,42,selectedColor);
    std::string display=inputs[field];
    if(c.width(display)>280){while(!display.empty()&&c.width("..."+display)>280)display.erase(0,1);display="..."+display;}
    c.text(16,184,display.empty()?"_":display,fg);
    const auto errors=c.wrap(error,304,true);
    for(size_t i=0;i<std::min<size_t>(2,errors.size());++i)c.text(8,218+static_cast<int>(i)*16,errors[i],red,true);
    for(int row=0;row<4;++row)for(int col=0;col<4;++col){const int x=7+col*78,y=250+row*58;
      c.rect(x,y,72,52,selectedColor);const std::string label=keypad[row][col];c.text(x+(72-c.width(label))/2,y+15,label,fg);}
    footer(c,"Abbruch","Weiter","Rechnen");
  }else{
    header(c,messageTitle,true);
    for(int i=0;i<20&&messageScroll+i<static_cast<int>(messageLines.size());++i)c.text(8,56+i*20,messageLines[messageScroll+i],fg);
    footer(c,"<",">","Zurueck");
  }
  dirty=false;
}
}

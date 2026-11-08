// SPDX-License-Identifier: GPL-3.0-or-later
#include "app.hpp"
#include <algorithm>
#include <cstdlib>

namespace physik {
namespace {
constexpr uint16_t bg=0xef9d,fg=0x1144,accent=0x2490,selectedColor=0xb6b6,white=0xffff,dim=0x6b6d;
constexpr uint32_t chapterIds[]={120,123,141};
constexpr const char* chapterNames[]={"Materiewellen / Beugung","Doppelspalt","Emission / Spektren"};
void header(Canvas& c,const std::string& title,bool back){
  c.rect(0,0,320,42,accent);if(back)c.text(8,10,"<",white);
  c.text(back?38:12,10,c.fit(title,back?274:298),white);
}
void footer(Canvas& c,const char* a,const char* b,const char* last){
  c.rect(0,482,320,46,selectedColor);c.text(25,495,a,fg);c.text(130,495,b,fg);c.text(223,495,last,fg);
}
}
void App::notify(std::string text,Page destination){message=std::move(text);messageBack=destination;page=Message;dirty=true;}
void App::activate(){
  if(page==Home){
    if(selected==4){running=false;return;}
    if(selected==0){page=Theory;selected=articleIndex;}
    else notify("Native Vorabversion: Dieser Bereich folgt nach dem ersten Geraetetest. Aktuell verfuegbar: Theorie mit drei Themen und den zugehoerigen Abbildungen.",Home);
  }else if(page==Theory){
    articleIndex=selected;
    std::string chapter;
    if(!store.chapter(chapterIds[articleIndex],chapter)||!store.images(chapterIds[articleIndex],ids)){notify(store.error(),Theory);return;}
    title=chapterNames[articleIndex];
    uint16_t pixel=0;Canvas measure(&pixel);lines=measure.wrap(chapter,304);
    scroll=0;page=Article;
  }else if(page==Article&&!ids.empty()){image=0;page=Image;}
  else if(page==Image||page==Message)back();
  dirty=true;
}
void App::back(){
  if(page==Home){running=false;return;}
  if(page==Image)page=Article;
  else if(page==Article){page=Theory;selected=articleIndex;}
  else if(page==Message)page=messageBack;
  else {page=Home;selected=0;}
  dirty=true;
}
void App::moveScroll(int delta){scroll=std::clamp(scroll+delta,0,std::max(0,static_cast<int>(lines.size())-20));dirty=true;}
void App::key(int code,bool repeat){
  if(!running|| (repeat&&(code==Enter||code==Back||code==Clear)))return;
  if(code==Back||code==Clear){back();return;}
  if(code==Enter){activate();return;}
  if(page==Article){moveScroll(code==Up?-1:code==Down?1:code==Left?-19:code==Right?19:0);return;}
  if(page==Image&&(code==Left||code==Right)){
    if(!ids.empty())image=(image+(code==Left?-1:1)+static_cast<int>(ids.size()))%static_cast<int>(ids.size());
    dirty=true;return;
  }
  if(page==Home||page==Theory){const int count=page==Home?5:3;selected=std::clamp(selected+(code==Up?-1:code==Down?1:0),0,count-1);dirty=true;}
}
void App::touchDown(int x,int y){if(x<0||x>=320||y<0||y>=528)return;touchX=x;touchY=y;startScroll=scroll;dragged=cancelled=false;}
void App::touchMove(int x,int y){
  if(touchX<0)return;
  if(x<0||x>=320||y<0||y>=528){cancelled=true;return;}
  if(page==Article&&touchY>=52&&touchY<482&&std::abs(y-touchY)>10){dragged=true;scroll=startScroll;moveScroll((touchY-y)/20);}
  else if(std::abs(x-touchX)>12||std::abs(y-touchY)>12)cancelled=true;
}
void App::touchUp(){if(touchX>=0&&!cancelled&&!dragged)tap(touchX,touchY);touchX=touchY=-1;}
void App::tap(int x,int y){
  if(y<42&&x<42&&page!=Home){back();return;}
  if(page==Home||page==Theory){const int index=(y-58)/48;if(y>=58&&index<(page==Home?5:3)){selected=index;activate();}return;}
  if(y<482)return;
  if(page==Article){if(x<105)moveScroll(-19);else if(x<210)moveScroll(19);else activate();}
  else if(page==Image){if(x<105)key(Left);else if(x<210)key(Right);else back();}
  else if(page==Message)back();
}
void App::draw(Canvas& c){
  c.rect(0,0,320,528,bg);
  if(page==Home||page==Theory){
    header(c,page==Home?"Physik LK Native":"Theorie",page!=Home);
    const int count=page==Home?5:3;
    for(int i=0;i<count;++i){const int y=58+i*48;if(i==selected)c.rect(6,y-3,308,44,selectedColor);
      c.text(12,y+8,c.fit(page==Home?homeItems[i]:chapterNames[i],296),fg);}
    c.text(10,395,"Native Vorabversion",dim);c.text(10,420,"Theorie und Bilder: 3 Themen",dim,true);
    c.text(10,442,"Andere Bereiche folgen spaeter.",dim,true);
    c.text(10,495,"EXE: oeffnen   Clear: zurueck",dim,true);
  }else if(page==Article){
    header(c,title,true);
    for(int i=0;i<20&&scroll+i<static_cast<int>(lines.size());++i)c.text(8,56+i*20,lines[scroll+i],fg);
    footer(c,"<",">", "Bilder");
  }else if(page==Image){
    Media media;std::vector<uint8_t> data;std::string error;
    if(image<0||image>=static_cast<int>(ids.size())||!store.media(ids[image],media)||!store.imageData(media,data)){
      notify(store.error()+"\nTheorie bleibt nutzbar.",Article);draw(c);return;
    }
    c.rect(0,0,320,528,white);header(c,media.title,true);
    c.text(142,45,std::to_string(image+1)+" / "+std::to_string(ids.size()),fg,true);
    if(!drawMedia(c,media,data,error)){notify(error+"\nTheorie bleibt nutzbar.",Article);draw(c);return;}
    const auto captions=c.wrap(media.caption,304);
    for(size_t i=0;i<std::min<size_t>(4,captions.size());++i)c.text(8,400+static_cast<int>(i)*20,captions[i],fg);
    footer(c,"<",">","Zurueck");
  }else {
    header(c,"Hinweis",true);const auto paragraphs=c.wrap(message,304);
    for(size_t i=0;i<std::min<size_t>(20,paragraphs.size());++i)c.text(8,56+static_cast<int>(i)*20,paragraphs[i],fg);
    footer(c,"","","Zurueck");
  }
  dirty=false;
}
}

// SPDX-License-Identifier: GPL-3.0-or-later
#include "canvas.hpp"
#include "font.hpp"
#include "small_font.hpp"
#include <algorithm>
#include <cstdlib>

namespace physik {
void Canvas::clip(int x,int y,int w,int h){
  x0_=std::clamp(x,0,screenWidth);y0_=std::clamp(y,0,screenHeight);
  x1_=std::clamp(x+w,x0_,screenWidth);y1_=std::clamp(y+h,y0_,screenHeight);
}
void Canvas::point(int x,int y,uint16_t color){if(x>=x0_&&x<x1_&&y>=y0_&&y<y1_)pixels[y*screenWidth+x]=color;}
void Canvas::rect(int x,int y,int w,int h,uint16_t color){
  const int right=std::min(x1_,x+w),bottom=std::min(y1_,y+h);
  for(int yy=std::max(y0_,y);yy<bottom;++yy)for(int xx=std::max(x0_,x);xx<right;++xx)pixels[yy*screenWidth+xx]=color;
}
void Canvas::line(int x,int y,int x2,int y2,uint16_t color){
  const int dx=std::abs(x2-x),sx=x<x2?1:-1,dy=-std::abs(y2-y),sy=y<y2?1:-1;
  int error=dx+dy;
  for(;;){point(x,y,color);if(x==x2&&y==y2)break;const int twice=2*error;
    if(twice>=dy){error+=dy;x+=sx;}if(twice<=dx){error+=dx;y+=sy;}}
}
int Canvas::width(const std::string& value,bool small)const{
  int result=0;for(unsigned char ch:value){
    if(small)result+=smallfont::advance[ch>=32&&ch<127?ch-32:'?'-32];
    else result+=classic::font::advance[ch>=32?ch-32:0];
  }return result;
}
void Canvas::text(int x,int y,const std::string& value,uint16_t color,bool small){
  const int height=small?smallfont::height:20;
  for(unsigned char ch:value){const int index=small?(ch>=32&&ch<127?ch-32:'?'-32):(ch>=32?ch-32:0);
    for(int yy=0;yy<height;++yy){const uint16_t bits=small?smallfont::bits[index][yy]:classic::font::bits[index][yy];
      for(int xx=0;xx<16;++xx)if(bits&(0x8000>>xx))point(x+xx,y+yy,color);}
    x+=small?smallfont::advance[index]:classic::font::advance[index];
  }
}
std::string Canvas::fit(std::string value,int available,bool small)const{
  if(width(value,small)<=available)return value;
  while(!value.empty()&&width(value+"...",small)>available)value.pop_back();
  return value+"...";
}
std::vector<std::string> Canvas::wrap(const std::string& value,int available,bool small)const{
  std::vector<std::string> result;std::string current,word;
  auto flush=[&](){
    if(word.empty())return;
    if(!current.empty()&&width(current+" "+word,small)>available){result.push_back(current);current.clear();}
    while(!word.empty()&&width(word,small)>available){size_t n=1;while(n<word.size()&&width(word.substr(0,n+1),small)<=available)++n;
      if(!current.empty()){result.push_back(current);current.clear();}result.push_back(word.substr(0,n));word.erase(0,n);}
    if(!word.empty()){if(!current.empty())current+=' ';current+=word;}word.clear();
  };
  for(char ch:value){if(ch==' '||ch=='\n'||ch=='\r'||ch=='\t'){flush();if(ch=='\n'){result.push_back(current);current.clear();}}else word+=ch;}
  flush();if(!current.empty())result.push_back(current);return result;
}
namespace {
bool number(const std::string& line,size_t& p,int& result){
  while(p<line.size()&&line[p]==' ')++p;
  bool minus=false;if(p<line.size()&&line[p]=='-'){minus=true;++p;}
  const size_t begin=p;int value=0;
  while(p<line.size()&&line[p]>='0'&&line[p]<='9'){if(p-begin>=6)return false;value=value*10+(line[p++]-'0');}
  if(begin==p||(p<line.size()&&line[p]!=' '))return false;
  result=minus?-value:value;while(p<line.size()&&line[p]==' ')++p;return true;
}
uint16_t word(const std::vector<uint8_t>& data,size_t position){return static_cast<uint16_t>((data[position]<<8)|data[position+1]);}
}
bool drawMedia(Canvas& canvas,const Media& media,const std::vector<uint8_t>& bytes,std::string& error,int x,int y){
  error.clear();
  if(media.width<1||media.width>304||media.height<1||media.height>320){error="Ungueltige Bildgroesse";return false;}
  Canvas view=canvas;view.clip(x,y,static_cast<int>(media.width),static_cast<int>(media.height));
  if(media.kind=="V"){
    if(bytes.empty()||!std::all_of(bytes.begin(),bytes.end(),[](uint8_t c){return c=='\n'||(c>=32&&c<127);})){error="Ungueltige Vektordaten";return false;}
    const std::string source(bytes.begin(),bytes.end());size_t begin=0;
    while(begin<source.size()){
      const size_t end=source.find('\n',begin);const std::string line=source.substr(begin,end==std::string::npos?end:end-begin);
      begin=end==std::string::npos?source.size():end+1;if(line.empty())continue;
      const bool text=line[0]=='T';const int count=text?3:5;int values[5]{};size_t p=2;
      if(line.size()<2||line[1]!=' '||(!text&&line[0]!='L')){error="Unbekannter Vektorbefehl";return false;}
      for(int i=0;i<count;++i)if(!number(line,p,values[i])){error="Ungueltiger Vektorbefehl";return false;}
      for(int i=0;i<count-1;++i)if(std::abs(values[i])>4096){error="Vektorkoordinate ausserhalb des Bereichs";return false;}
      if(values[count-1]<0||values[count-1]>65535||(!text&&p!=line.size())){error="Ungueltige Vektorfarbe";return false;}
      const uint16_t color=static_cast<uint16_t>(values[count-1]&0xffdf);
      if(text)view.text(x+values[0],y+values[1],line.substr(p),color,true);
      else view.line(x+values[0],y+values[1],x+values[2],y+values[3],color);
    }
    return true;
  }
  if(media.kind!="P4"&&media.kind!="P8"){error="Unbekanntes Rasterformat";return false;}
  const uint32_t count=media.kind=="P4"?16:256,stride=media.kind=="P4"?(media.width+1)/2:media.width;
  if(bytes.size()!=2*count+stride*media.height){error="Ungueltige Rasterlaenge";return false;}
  for(uint32_t yy=0;yy<media.height;++yy)for(uint32_t xx=0;xx<media.width;++xx){
    const uint8_t value=bytes[2*count+yy*stride+(media.kind=="P4"?xx/2:xx)];
    const uint32_t index=media.kind=="P4"?(xx%2?value&15:value>>4):(value+128u)&255u;
    view.point(x+static_cast<int>(xx),y+static_cast<int>(yy),word(bytes,index*2));
  }
  return true;
}
}

// SPDX-License-Identifier: GPL-3.0-or-later
#include "plot_view.hpp"
#include <algorithm>
#include <cmath>
#include <limits>
#include <numeric>

namespace physik {
namespace {
constexpr int left=60,right=305,top=96,bottom=366;
constexpr uint16_t ink=0x1144,grid=0xce79,curve=0x2490,red=0xc100;
double fraction(double x,double a,double b){
  if(x<=a)return 0;
  if(x>=b)return 1;
  const double span=b-a;
  return std::isfinite(span)?(x-a)/span:(x/2-a/2)/(b/2-a/2);
}
int px(double x,const PlotBounds& b){return left+static_cast<int>(std::round(std::clamp(fraction(x,b.xmin,b.xmax),0.0,1.0)*(right-left)));}
int py(double y,const PlotBounds& b){return bottom-static_cast<int>(std::round(std::clamp(fraction(y,b.ymin,b.ymax),0.0,1.0)*(bottom-top)));}
bool inside(double x,double y,const PlotBounds& b){return x>=b.xmin&&x<=b.xmax&&y>=b.ymin&&y<=b.ymax;}
using Point=std::pair<double,double>;
unsigned outside(Point p,const PlotBounds& b){
  return (p.first<b.xmin?1u:p.first>b.xmax?2u:0u)|
         (p.second<b.ymin?4u:p.second>b.ymax?8u:0u);
}
Point midpoint(Point a,Point b){return {std::midpoint(a.first,b.first),std::midpoint(a.second,b.second)};}
Point edge(Point outer,Point inner,const PlotBounds& b){
  // Binary64's entire exponent range plus its significand fits in this bound.
  for(unsigned i=0;i<2150;++i){
    const Point mid=midpoint(outer,inner);
    if(mid==outer||mid==inner)break;
    if(outside(mid,b))outer=mid;else inner=mid;
  }
  return inner;
}
bool segment(Point& a,Point& z,const PlotBounds& b){
  if(!std::isfinite(a.first)||!std::isfinite(a.second)||
     !std::isfinite(z.first)||!std::isfinite(z.second))return false;
  unsigned ca=outside(a,b),cz=outside(z,b);
  if(ca&cz)return false;
  Point inner;
  bool found=false;
  if(!ca){inner=a;found=true;}else if(!cz){inner=z;found=true;}
  // Subdivide in world coordinates: no overflowing endpoint difference or
  // unrepresentable interpolation fraction for a tiny crop of a huge line.
  for(unsigned i=0;!found&&i<2150;++i){
    const Point mid=midpoint(a,z);
    if(mid==a||mid==z)return false;
    const unsigned code=outside(mid,b);
    if(!code){inner=mid;found=true;break;}
    if(code&ca){a=mid;ca=code;}else {z=mid;cz=code;}
    if(ca&cz)return false;
  }
  if(!found)return false;
  if(ca)a=edge(a,inner,b);
  if(cz)z=edge(z,inner,b);
  return true;
}
void series(Canvas& c,const std::vector<std::pair<double,double>>& points,const PlotBounds& b,bool joined,uint16_t color){
  for(size_t i=0;i<points.size();++i){auto [x,y]=points[i];
    if(joined&&i){Point a=points[i-1],z={x,y};
      if(segment(a,z,b))c.line(px(a.first,b),py(a.second,b),px(z.first,b),py(z.second,b),color);
    }
    if(inside(x,y,b)){const int a=px(x,b),z=py(y,b);c.rect(a-1,z-1,3,3,color);}
  }
}
}
bool validPlotBounds(const PlotBounds& b){
  return std::isfinite(b.xmin)&&std::isfinite(b.xmax)&&std::isfinite(b.ymin)&&std::isfinite(b.ymax)&&b.xmin<b.xmax&&b.ymin<b.ymax;
}
bool plotBounds(const Plot& p,PlotBounds& b,std::string& error){
  error.clear();bool first=true;
  for(const auto* values:{&p.points,&p.model})for(auto [x,y]:*values){
    if(!std::isfinite(x)||!std::isfinite(y)){error="Nicht endliche Diagrammpunkte";return false;}
    if(first){b={x,x,y,y};first=false;}
    else {b.xmin=std::min(b.xmin,x);b.xmax=std::max(b.xmax,x);b.ymin=std::min(b.ymin,y);b.ymax=std::max(b.ymax,y);}
  }
  if(first){error="Diagramm ohne Punkte";return false;}
  auto widen=[](double& lo,double& hi){
    if(lo==hi){double pad=std::max(std::abs(lo)*.1,1e-100);lo-=pad;hi+=pad;}
    else {const double pad=(hi/20-lo/20);const double a=lo-pad,z=hi+pad;if(std::isfinite(a)&&std::isfinite(z)){lo=a;hi=z;}}
  };
  widen(b.xmin,b.xmax);widen(b.ymin,b.ymax);
  if(!validPlotBounds(b)){error="Achsenbereich nicht darstellbar";return false;}return true;
}
size_t plotCursor(const Plot& p,const PlotBounds& b,int x){
  size_t best=p.points.size();int distance=std::numeric_limits<int>::max();
  if(!validPlotBounds(b))return best;
  x=std::clamp(x,left,right);
  for(size_t i=0;i<p.points.size();++i){
    const auto [a,z]=p.points[i];if(!inside(a,z,b))continue;
    const int d=std::abs(px(a,b)-x);if(d<distance){distance=d;best=i;}
  }
  return best;
}
void drawPlot(Canvas& c,const Plot& p,const PlotBounds& b,size_t cursor){
  if(!validPlotBounds(b))return;
  c.rect(left,top,right-left+1,bottom-top+1,0xffff);
  for(int i=0;i<=4;++i){const int x=left+i*(right-left)/4,y=bottom-i*(bottom-top)/4;c.line(x,top,x,bottom,grid);c.line(left,y,right,y,grid);}
  if(b.xmin<=0&&b.xmax>=0)c.line(px(0,b),top,px(0,b),bottom,ink);
  if(b.ymin<=0&&b.ymax>=0)c.line(left,py(0,b),right,py(0,b),ink);
  series(c,p.points,b,p.joined,curve);series(c,p.model,b,true,red);
  c.line(left,bottom,right,bottom,ink);c.line(left,top,left,bottom,ink);
  c.text(8,52,c.fit(p.yLabel+" ["+p.yUnit+"]",304),ink,true);
  c.text(60,72,c.fit("max "+formatNumber(b.ymax),240),ink,true);
  c.text(8,374,c.fit("y min "+formatNumber(b.ymin),304),ink,true);
  c.text(8,391,c.fit("x "+formatNumber(b.xmin)+" ... "+formatNumber(b.xmax),304),ink,true);
  c.text(8,408,c.fit(p.xLabel+" ["+p.xUnit+"]",304),ink,true);
  if(cursor<p.points.size()){
    const auto [x,y]=p.points[cursor];
    if(inside(x,y,b)){const int a=px(x,b),z=py(y,b);c.line(a,top,a,bottom,red);c.rect(a-3,z-3,7,7,red);}
    c.text(8,430,c.fit(std::to_string(cursor+1)+"/"+std::to_string(p.points.size())+" x="+formatNumber(x),304),ink,true);
    c.text(8,447,c.fit("y="+formatNumber(y)+"  "+p.yUnit,304),ink,true);
  }
  c.text(8,465,"EXE: Tabelle; A: Achsen; S: Auto",ink,true);
}
}

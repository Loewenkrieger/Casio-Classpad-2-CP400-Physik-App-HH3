// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "store.hpp"
#include <cstdint>

namespace physik {
inline constexpr int screenWidth=320,screenHeight=528;
class Canvas {
public:
  explicit Canvas(uint16_t* buffer):pixels(buffer){}
  uint16_t* pixels;
  void clip(int x,int y,int width,int height);
  void point(int x,int y,uint16_t color);
  void rect(int x,int y,int width,int height,uint16_t color);
  void line(int x,int y,int x2,int y2,uint16_t color);
  void text(int x,int y,const std::string& value,uint16_t color,bool small=false);
  int width(const std::string& text,bool small=false) const;
  std::string fit(std::string text,int width,bool small=false) const;
  std::vector<std::string> wrap(const std::string& text,int width,bool small=false) const;
private:
  int x0_=0,y0_=0,x1_=screenWidth,y1_=screenHeight;
};
bool drawMedia(Canvas& canvas,const Media& media,const std::vector<uint8_t>& bytes,
               std::string& error,int x=8,int y=68);
}

// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "canvas.hpp"

namespace physik {
enum Key { Up=256,Down,Left,Right,Enter,Back,Clear };
class App {
public:
  enum Page { Home,Theory,Article,Image,Message };
  explicit App(std::string directory):store(std::move(directory)){}
  bool running=true,dirty=true;
  Page page=Home;
  int selected=0,scroll=0,image=0;
  Store store;
  void key(int code,bool repeat=false);
  void touchDown(int x,int y);
  void touchMove(int x,int y);
  void touchUp();
  void draw(Canvas& canvas);
  static constexpr const char* homeItems[]={"Theorie","Formeln nach Groessen","Formeln nach Themen","Physik-Rechner","Beenden"};
private:
  int articleIndex=0,touchX=-1,touchY=-1,startScroll=0;
  bool dragged=false,cancelled=false;
  std::vector<std::string> lines,ids;
  std::string message,title;
  Page messageBack=Home;
  void activate();
  void back();
  void tap(int x,int y);
  void notify(std::string text,Page destination);
  void moveScroll(int delta);
};
}

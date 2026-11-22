// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "canvas.hpp"
#include "calculator.hpp"
#include "kinematics.hpp"
#include "extensions.hpp"
#include "catalog_data.hpp"
#include "plot_view.hpp"
#include "navigation.hpp"

namespace physik {
enum Key { Up=256,Down,Left,Right,Enter,Back,Clear };
class App {
public:
  enum Page { Home,Theory,Entries,CalcCategories,Calcs,Article,Image,Form,Message,Chart,TableView };
  explicit App(std::string directory):store(directory){nativeDataDirectory(directory);}
  bool running=true,dirty=true;
  Page page=Home;
  int selected=0,scroll=0,image=0,field=0;
  Store store;
  void key(int code,bool repeat=false);
  void touchDown(int x,int y);
  void touchMove(int x,int y);
  void touchUp();
  void draw(Canvas& canvas);
  std::vector<std::string> labels()const;
  const std::string& messageText()const{return message;}
#ifndef PHYSIK_HH3
  size_t loadedNavigationLevels()const;
#endif
  enum HomeMenu {TheorySearch,TopicSearch,AlphabetSearch,CalculatorMenu,ExitMenu,HomeCount};
  static constexpr const char* homeItems[]={"Theorie","Formeln nach Themen","Formeln A-Z","Physik-Rechner","Beenden"};
private:
  struct Menu {Page page;std::string title;std::vector<Value> rows;int selected=0,first=0;bool native=false,navigation=false;uint32_t node=UINT32_MAX;};
  std::vector<Menu> menus;
  int homeSelection=0,first=0,touchX=-1,touchY=-1,startScroll=0;
  bool dragged=false,cancelled=false,replaceOnType=false;
  std::vector<std::string> lines,ids,messageLines,inputs;
  std::vector<size_t> units;
  std::vector<Parameter> parameters;
  Calculation calculation;
  bool fitting=false,wavelength=false,variableCount=false;
  bool extended=false,axisEditing=false;
  Workflow workflow;
  std::vector<Plot> plots;
  size_t plotIndex=0,cursor=0;
  int tableScroll=0,savedField=0;
  PlotBounds bounds;
  std::vector<PlotBounds> plotLimits;
  std::vector<Parameter> savedParameters;
  std::vector<std::string> savedInputs;
  std::vector<size_t> savedUnits;
  int pointCount=0,messageScroll=0;
  std::string message,title,messageTitle,error;
  Page messageBack=Home;
  void activate();
  void back();
  void tap(int x,int y);
  void notify(std::string text,Page destination,const std::string& heading="Hinweis");
  void moveScroll(int delta);
  bool openMenu(Page kind,const std::string& title,Table table,uint32_t start,uint32_t count);
  bool showMenu(Page kind,const std::string& title,std::vector<Value> rows,bool native=false,bool navigation=false);
  void openNavigation(uint32_t node,const std::string& heading);
  bool loadNavigation(uint32_t node,std::vector<Value>& rows,Page& kind);
  void openKinematics();
  void openNative(const std::string& category,const std::string& heading);
  void openPlot(bool autoscale=false);
  void nextPlot(int direction);
  void openAxes();
  void restoreForm();
  void expandNative(unsigned count);
  std::vector<Unit> fieldUnits(size_t index)const;
  void ensureVisible();
  void openArticle(const Value& value);
  void beginCalculation(const Value& value);
  void fitParameters(int count);
  bool validateField(double& value);
  void nextField(int direction);
  void compute();
  void edit(char key);
  void changeUnit();
};
}

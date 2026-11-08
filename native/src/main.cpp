// SPDX-License-Identifier: GPL-3.0-or-later
#include "app.hpp"
#include "screen_backup.hpp"
#include <appdef.h>
#include <sdk/os/input.h>
#include <sdk/os/lcd.h>

APP_NAME("Physik LK Native")
APP_DESCRIPTION("Native Physik-App: Theorie- und Bildprototyp fuer HH3.")
APP_AUTHOR("Loewenkrieger")
APP_VERSION("2.0.0-alpha.1")

int main(){
  if(!physik::screenReady())return 1;
  unsigned int w=0,h=0;LCD_GetSize(&w,&h);
  if(w!=physik::screenWidth||h!=physik::screenHeight)return 1;
  physik::Canvas canvas(LCD_GetVRAMAddress());
  physik::App app("\\\\fls0\\");
  while(app.running){
    if(app.dirty){app.draw(canvas);LCD_Refresh();}
    alignas(4) Input_Event event{};GetInput(&event,0xffffffff,0x10);
    if(event.type==EVENT_ACTBAR_ESC){app.key(physik::Back);continue;}
    if(event.type==EVENT_KEY&&(event.data.key.direction==KEY_PRESSED||event.data.key.direction==KEY_HELD)){
      int key=0;switch(event.data.key.keyCode){
        case KEYCODE_UP:key=physik::Up;break;case KEYCODE_DOWN:key=physik::Down;break;
        case KEYCODE_LEFT:key=physik::Left;break;case KEYCODE_RIGHT:key=physik::Right;break;
        case KEYCODE_EXE:key=physik::Enter;break;case KEYCODE_POWER_CLEAR:key=physik::Clear;break;
        case KEYCODE_BACKSPACE:key=physik::Back;break;default:break;
      }
      if(key)app.key(key,event.data.key.direction==KEY_HELD);
    }else if(event.type==EVENT_TOUCH){const auto touch=event.data.touch_single;
      if(touch.direction==TOUCH_DOWN)app.touchDown(touch.p1_x,touch.p1_y);
      else if(touch.direction==TOUCH_HOLD_DRAG)app.touchMove(touch.p1_x,touch.p1_y);
      else if(touch.direction==TOUCH_UP)app.touchUp();
    }
  }
  return 0;
}

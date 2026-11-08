// SPDX-License-Identifier: GPL-3.0-or-later
#include "screen_backup.hpp"
#include <sdk/calc/calc.h>
#include <sdk/os/lcd.h>
#include <cstdlib>
namespace { constinit physik::ScreenBackup originalScreen; }
extern "C" __attribute__((used,noinline,externally_visible)) void calcInit(){originalScreen.save(vram,std::malloc,std::free);}
extern "C" __attribute__((used,noinline,externally_visible)) void calcExit(){if(originalScreen.restore())LCD_Refresh();}
namespace physik { bool screenReady(){return originalScreen.ready();} }

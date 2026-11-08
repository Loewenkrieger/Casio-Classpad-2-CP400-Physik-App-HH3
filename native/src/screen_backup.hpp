// SPDX-License-Identifier: GPL-3.0-or-later
// Adapted from Mathe Vektor, Loewenkrieger.
#pragma once
#include <cstddef>
namespace physik {
// The code occupies VBAK; the saved screen must live on the OS heap.
class ScreenBackup {
public:
  static constexpr std::size_t bytes=320u*528u*2u;
  using Allocate=void* (*)(std::size_t);
  using Release=void (*)(void*);
  constexpr ScreenBackup()=default;
  ScreenBackup(const ScreenBackup&)=delete;
  ScreenBackup& operator=(const ScreenBackup&)=delete;
  bool save(void* screen,Allocate allocate,Release release);
  bool restore();
  bool ready()const{return saved_!=nullptr;}
private:
  void* saved_=nullptr;
  void* screen_=nullptr;
  Release release_=nullptr;
};
bool screenReady();
}

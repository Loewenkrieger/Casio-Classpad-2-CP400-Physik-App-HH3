// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "extensions.hpp"

namespace physik {
std::vector<NativeCategory> supplementSections(int mode);
std::vector<std::pair<std::string,uint32_t>> supplementEntries(const std::string& category);
bool supplementLoad(Store& store,uint32_t index,std::string& text,std::vector<std::string>& media,std::string& error);
size_t supplementCount();
size_t supplementLinkCount();
}

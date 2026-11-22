// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "store.hpp"

namespace physik {
enum class NavigationKind : uint32_t { Menu=0, LegacyArticle=1, NativeArticle=2 };
struct NavigationItem {std::string label;NavigationKind kind;uint32_t target;};
bool navigationRows(Store& store,uint32_t node,std::vector<NavigationItem>& rows,std::string& error);
uint32_t navigationNodeCount();
uint32_t navigationRoot(unsigned mode);
}

// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "extensions.hpp"

namespace physik {
void nativeDataDirectory(const std::string& directory);
const std::string& nativeCatalogError();
#ifdef PHYSIK_DATA_CATALOG
std::vector<Workflow> dataCatalog(const std::string& category);
bool dataWorkflow(const std::string& id,Workflow& result);
#endif
}

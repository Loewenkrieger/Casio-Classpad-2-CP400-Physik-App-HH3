// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "navigation.hpp"

namespace browse_test {
inline bool find(physik::Store& store,uint32_t node,physik::NavigationKind kind,uint32_t target,
                 std::vector<int>& path,std::string& error,unsigned depth=0,int topic=-1){
  if(depth>3){error="Navigation deeper than the approved hierarchy";return false;}
  std::vector<physik::NavigationItem> rows;
  if(!physik::navigationRows(store,node,rows,error))return false;
  for(size_t i=0;i<rows.size();++i){
    if(depth==0&&topic>=0&&static_cast<int>(i)!=topic)continue;
    const auto& row=rows[i];path.push_back(static_cast<int>(i));
    if(row.kind==kind&&row.target==target)return true;
    if(row.kind==physik::NavigationKind::Menu&&find(store,row.target,kind,target,path,error,depth+1))return true;
    path.pop_back();if(!error.empty())return false;
  }
  return false;
}
}

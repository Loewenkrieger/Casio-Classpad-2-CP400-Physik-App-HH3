// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <algorithm>
#include <string>

namespace physik {
inline std::string dataDirectory(const char* executable){
  const std::string root="\\fls0\\";
  if(!executable)return root;
  size_t length=0;
  while(length<512&&executable[length])++length;
  if(length==512)return root;
  std::string path(executable,length);
  std::replace(path.begin(),path.end(),'/','\\');
  while(path.size()>1&&path[0]=='\\'&&path[1]=='\\')path.erase(0,1);
  if(path.rfind(root,0)!=0)return root;
  const auto last=path.find_last_of('\\');
  return path.substr(0,last+1);
}
}

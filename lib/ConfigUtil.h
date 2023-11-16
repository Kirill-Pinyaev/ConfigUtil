#ifndef _CONFIG_UTIL_H_
#define _CONFIG_UTIL_H_

#include <cmath>
#include <iostream>
#include <vector>

class ConfigUtil {
 private:
  std::vector<std::string> data;

 public:
  // ConfigUtil();
  int SplitString(int argc, char** argv);
  int PrintString();

  //     ~ConfigUtil();
};

#endif
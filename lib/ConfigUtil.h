#ifndef _CONFIG_UTIL_H_
#define _CONFIG_UTIL_H_

#include <string.h>

#include <algorithm>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <vector>

class ConfigUtil {
 private:
  std::vector<std::string> data;
  std::vector<std::string> comand = {"read", "write", "import", "export"};

 public:
  // ConfigUtil();
  int SplitString(int argc, char** argv);
  int PrintString();

  //     ~ConfigUtil();
};

class Errors {
 public:
  Errors(std::string message) : message{message} {}
  std::string getMessage() const { return message; }

 private:
  std::string message;
};
#endif
#ifndef _ARGUMENTS_H_
#define _ARGUMENTS_H_

#include <string.h>

#include <algorithm>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <vector>

#include "lib/sqlite/sqlite3.h"

class Arguments {
 private:
  std::vector<std::string> data;
  std::vector<std::string> comandlist = {"read", "write", "import", "export"};

 public:
  std::string vault = "config.sqlite";
  int flagVault = 0;
  std::string command;
  std::vector<std::string> params;
  Arguments(int argc, char** argv);
};
#endif
#include "ConfigUtil.h"

int ConfigUtil::SplitString(int argc, char* argv[]) {
  for (int i = 0; i < argc; i++) {
    data.push_back(argv[i]);
  }
  return 1;
};

int ConfigUtil::PrintString() {
  for (auto i : data) {
    std::cout << i << std::endl;
  }
  return 1;
}

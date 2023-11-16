#include "ConfigUtil.h"

int ConfigUtil::SplitString(int argc, char* argv[]) {
  try {
    for (int i = 1; i < argc; i++) {
      if (i == 2) {
        if (std::find(comand.begin(), comand.end(), argv[i]) == comand.end()) {
          throw Errors("ERROR: invalid argument " + std::string(argv[i]));
        }
      }
      data.push_back(argv[i]);
    }
  } catch (const Errors& e) {
    std::cout << e.getMessage() << std::endl;
    return 1;
  };
  return 0;
};

int ConfigUtil::PrintString() {
  for (auto i : data) {
    std::cout << i << std::endl;
  }
  return 1;
}

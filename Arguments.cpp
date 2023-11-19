#include "Arguments.h"

int Arguments::SplitString(int argc, char* argv[]) {
  try {
    for (int i = 1; i < argc; i++) {
      if (i == 1) {
        if (std::find(comandlist.begin(), comandlist.end(), argv[i]) !=
            comandlist.end()) {
          command = argv[i];
        } else {
          vault = argv[i];
        }
      } else if (i == 2 && command.empty()) {
        if (std::find(comandlist.begin(), comandlist.end(), argv[i]) !=
            comandlist.end()) {
          command = argv[i];
        } else {
          throw std::runtime_error("invalid command: " + std::string(argv[i]));
        }
      } else {
        params.push_back(argv[i]);
      }
      data.push_back(argv[i]);
    }
  } catch (const std::runtime_error& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  };
  return 0;
};
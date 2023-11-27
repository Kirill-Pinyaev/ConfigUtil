#include "Arguments.h"

Arguments::Arguments(int argc, char** argv) {
  for (int i = 1; i < argc; i++) {
    if (i == 1) {
      if (std::find(comandlist.begin(), comandlist.end(), argv[i]) !=
          comandlist.end()) {
        command = argv[i];
      } else {
        vault = argv[i];
        flagVault = 1;
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
  if (command.empty()) {
    throw std::runtime_error("No command");
  }
};

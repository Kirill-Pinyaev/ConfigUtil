#include "Arguments.h"

int main(int argc, char* argv[]) {
  Arguments args(argc, argv);
  std::cout << "Vault: " << args.vault << std::endl;
  std::cout << "Command: " << args.command << std::endl;
  std::cout << "Args: ";
  for (auto param : args.params) {
    std::cout << param << " ";
  }
  return 0;
}
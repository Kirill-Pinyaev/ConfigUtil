#include "Arguments.h"

int main(int argc, char* argv[]) {
  try {
    Arguments args(argc, argv);
    std::cout << "Vault: " << args.vault << std::endl;
    std::cout << "Command: " << args.command << std::endl;
    std::cout << "Args: ";
    for (auto param : args.params) {
      std::cout << param << " ";
    }
  } catch (const std::runtime_error& e) {
    std::cerr << "Error: " << e.what() << std::endl;
  }
  return 0;
}
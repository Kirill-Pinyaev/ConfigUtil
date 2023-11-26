#include "Arguments.h"
#include "SqlDatabase.h"

int main(int argc, char* argv[]) {
  try {
    Arguments args(argc, argv);
    SqlDatabase db(args.vault);
    std::cout << "Vault: " << args.vault << std::endl;
    std::cout << "Command: " << args.command << std::endl;
    std::cout << "Args: ";
    for (auto param : args.params) {
      std::cout << param << " ";
    }
    std::cout << std::endl;
    db.CreateDatabase();

  } catch (const std::runtime_error& e) {
    std::cerr << "Error: " << e.what() << std::endl;
  } catch (const std::logic_error& e) {
    std::cerr << "Error: Database error: " << e.what() << std::endl;
  }
  return 0;
}
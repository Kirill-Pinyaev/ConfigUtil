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
    if (!(args.params.empty())) {
      if (args.command == "read") {
        db.CreateDatabase();
        db.ReadDatabase(args.params[0]);
      } else if (args.command == "write" && args.params.size() >= 2) {
        db.CreateDatabase();
        db.WriteToDatabase(args.params[0], args.params[1]);
      } else if (args.command == "import") {
        db.CreateDatabase();
        std::cout << "import" << std::endl;
      } else if (args.command == "export") {
        db.CreateDatabase();
        std::cout << "export" << std::endl;
      }
    }
  } catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
  }
  return 0;
}
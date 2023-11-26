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
    if (args.command == "read") {
      db.ReadDatabase(args.params[0]);
    } else if (args.command == "write") {
      db.WriteToDatabase(args.params[0], args.params[1]);
    } else if (args.command == "import") {
      std::cout << "import" << std::endl;
    } else if (args.command == "export") {
      std::cout << "export" << std::endl;
    }
  } catch (const std::runtime_error& e) {
    std::cerr << "Error: " << e.what() << std::endl;
  } catch (const std::logic_error& e) {
    std::cerr << "Error: Database error: " << e.what() << std::endl;
  }
  return 0;
}
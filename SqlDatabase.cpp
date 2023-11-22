#include "SqlDatabase.h"

SqlDatabase::SqlDatabase(std::string vault) {
  int resultOpen = sqlite3_open(("../" + vault).c_str(), &db);
  if (resultOpen != SQLITE_OK) {
    throw std::runtime_error(sqlite3_errmsg(db));
  }
  std::string request =
      "SELECT name FROM sqlite_master WHERE type='table' AND name='data';";
  sqlite3_stmt* statement;
  int resultRequest =
      sqlite3_prepare_v2(db, request.c_str(), -1, &statement, nullptr);

  if (resultRequest != SQLITE_OK) {
    throw std::runtime_error(sqlite3_errmsg(db));
  }

  resultRequest = sqlite3_step(statement);

  sqlite3_finalize(statement);
  if (!(resultRequest == SQLITE_ROW)) {
    const char* createTableQuery =
        "CREATE TABLE data ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "name TEXT NOT NULL,"
        "age INTEGER);";

    int resultCreate = sqlite3_exec(db, createTableQuery, 0, 0, 0);

    if (resultCreate != SQLITE_OK) {
      throw std::runtime_error(sqlite3_errmsg(db));
    }

    std::cout << "The database and table have been created successfully!"
              << std::endl;
  } else {
    std::cout << "Database opened" << std::endl;
  }

  sqlite3_close(db);
}
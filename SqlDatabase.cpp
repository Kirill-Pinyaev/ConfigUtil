#include "SqlDatabase.h"

SqlDatabase::SqlDatabase(std::string& vault) {
  int resultOpen = sqlite3_open(("../" + vault).c_str(), &db);
  if (resultOpen != SQLITE_OK) {
    throw std::logic_error("Opening or creating a database failed!");
  }
};

std::string SqlDatabase::ExecuteQuery(std::string& query) {
  sqlite3_stmt* stmt;

  int rc = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr);

  if (rc != SQLITE_OK) {
    // sqlite3_close(db);
    throw std::logic_error("Request failed: " +
                           std::string(sqlite3_errmsg(db)));
  }

  rc = sqlite3_step(stmt);

  if (rc != SQLITE_DONE && rc != SQLITE_ROW) {
    // sqlite3_finalize(stmt);
    // sqlite3_close(db);
    throw std::logic_error("Failed to execute query: " +
                           std::string(sqlite3_errmsg(db)));
  }

  std::string result;
  if (rc == SQLITE_ROW) {
    // printf("sadasd\n");
    result = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
  }

  sqlite3_finalize(stmt);

  return result;
}
int SqlDatabase::CreateDatabase() {
  std::string request =
      "SELECT name FROM sqlite_master WHERE type='table' AND name='data';";
  std::string result = ExecuteQuery(request);

  if (result.empty()) {
    const char* createTableQuery =
        "CREATE TABLE data ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "name TEXT NOT NULL);";

    int resultCreate = sqlite3_exec(db, createTableQuery, 0, 0, 0);

    if (resultCreate != SQLITE_OK) {
      throw std::logic_error("Creating a table failed: " +
                             std::string(sqlite3_errmsg(db)));
    }

    std::cout << "The database and table have been created successfully!"
              << std::endl;
  } else {
    std::cout << "Database opened" << std::endl;
  }

  return 0;
};

int SqlDatabase::ReadDatabase(std::string& key) {
  std::string sqlQuery = "SELECT name FROM data WHERE id = '" + key + "';";

  std::string result = ExecuteQuery(sqlQuery);
  std::cout << result << std::endl;
  return 0;
};

int SqlDatabase::WriteToDatabase(std::string& key, std::string& value) {
  std::string sqlQuery =
      "INSERT OR REPLACE INTO data (id, name) VALUES "
      "('" +
      key + "', '" + value + "');";
  std::string result = ExecuteQuery(sqlQuery);
  return 0;
};
#include "SqlDatabase.h"

SqlDatabase::SqlDatabase(std::string& vault) {
  int resultOpen = sqlite3_open(("../" + vault).c_str(), &db);
  if (resultOpen != SQLITE_OK) {
    throw std::logic_error("Opening or creating a database failed!");
  }
};

SqlDatabase::~SqlDatabase() { sqlite3_close(db); };

int SqlDatabase::SplitKey(std::string& key) {
  std::string part;
  std::istringstream tokenStream(key);
  while (std::getline(tokenStream, part, '.')) {
    paramGroup.push_back(part);
  }
  return 1;
};

std::string SqlDatabase::ExecuteQuery(std::string& query) {
  sqlite3_stmt* stmt;

  int rc = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr);

  if (rc != SQLITE_OK) {
    throw std::logic_error("Request failed: " +
                           std::string(sqlite3_errmsg(db)));
  }

  rc = sqlite3_step(stmt);

  if (rc != SQLITE_DONE && rc != SQLITE_ROW) {
    sqlite3_finalize(stmt);
    throw std::logic_error("Failed to execute query: " +
                           std::string(sqlite3_errmsg(db)));
  }

  std::string result;
  if (rc == SQLITE_ROW) {
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
        "GroupName TEXT,"
        "ParentGroup TEXT,"
        "ParameterName TEXT,"
        "ParameterValue TEXT,"
        "PRIMARY KEY (GroupName, ParameterName));";

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
  SplitKey(key);
  std::string groupName;
  std::string paramName;
  if (paramGroup.size() >= 2) {
    groupName = paramGroup.rbegin()[1];
    paramName = paramGroup.back();
  } else {
    groupName = "";
    paramName = paramGroup[0];
  }
  std::string sqlQuery = "SELECT ParameterValue FROM data WHERE GroupName = '" +
                         groupName + "' AND ParameterName = '" + paramName +
                         "';";

  std::string result = ExecuteQuery(sqlQuery);
  std::cout << result << std::endl;
  return 0;
};

int SqlDatabase::WriteToDatabase(std::string& key, std::string& value) {
  SplitKey(key);
  std::string groupName;
  std::string parentGroup;
  std::string paramName;
  if (paramGroup.size() >= 3) {
    parentGroup = paramGroup.rbegin()[2];
    groupName = paramGroup.rbegin()[1];
    paramName = paramGroup.back();
  } else if (paramGroup.size() == 2) {
    groupName = paramGroup[0];
    parentGroup = "";
    paramName = paramGroup[1];
  } else {
    groupName = "";
    parentGroup = "";
    paramName = paramGroup[0];
  }
  std::string sqlQuery =
      "INSERT INTO data (GroupName, ParentGroup, ParameterName, "
      "ParameterValue) VALUES ('" +
      groupName + "', '" + parentGroup + "', '" + paramName + "', '" + value +
      "');";
  ExecuteQuery(sqlQuery);
  std::cout << "Recording is successful" << std::endl;
  return 0;
};
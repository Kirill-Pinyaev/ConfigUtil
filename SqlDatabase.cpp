#include "SqlDatabase.h"

SqlDatabase::SqlDatabase(std::string& vault) {
  int resultOpen = sqlite3_open(vault.c_str(), &db);
  if (resultOpen != SQLITE_OK) {
    throw std::logic_error("Opening or creating a database failed!");
  }
};

SqlDatabase::~SqlDatabase() { sqlite3_close(db); };

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
    const char* text =
        reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
    result = text ? text : "";
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
        "Name TEXT,"
        "Value TEXT);";

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
  std::string sqlQuery = "SELECT Value FROM data WHERE Name = '" + key + "';";
  std::string resultRequest = ExecuteQuery(sqlQuery);
  std::cout << resultRequest << std::endl;
  return 0;
};

int SqlDatabase::WriteToDatabase(std::string& key, std::string& value) {
  std::string sqlQuery = "SELECT 1 FROM data WHERE Name = '" + key + "';";
  std::string resultRequest = ExecuteQuery(sqlQuery);
  if (resultRequest.empty()) {
    sqlQuery = "INSERT INTO data (Name, Value) VALUES ('" + key + "', '" +
               value + "');";
    ExecuteQuery(sqlQuery);
  } else {
    sqlQuery =
        "UPDATE data SET Value = '" + value + "' WHERE Name = '" + key + "';";
    ExecuteQuery(sqlQuery);
  }
  return 0;
};

int SqlDatabase::ExportDatabase(std::string& path) {
  std::vector<json> dataJson;
  json jsonData;

  const char* selectQuery = "SELECT * FROM data";
  sqlite3_stmt* stmt;

  int rc = sqlite3_prepare_v2(db, selectQuery, -1, &stmt, nullptr);
  if (rc != SQLITE_OK) {
    throw std::logic_error("Request failed: " +
                           std::string(sqlite3_errmsg(db)));
  }

  while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
    const char* name =
        reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
    const char* value =
        reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
    if (strchr(name, '.') != nullptr) {
      std::string nameGroup = GetPeriod(name, 0, 1);
      nameGroup = nameGroup.erase(nameGroup.size() - 1);
      if (jsonData.find(nameGroup) == jsonData.end()) {
        jsonData[nameGroup] = FillGroup(1, name);
      }
    } else {
      jsonData[name] = value;
    }
  }
  if (rc != SQLITE_DONE) {
    throw std::logic_error("Request failed: " +
                           std::string(sqlite3_errmsg(db)));
  }
  std::ofstream outFile(path);
  outFile << std::setw(4) << jsonData << std::endl;
  outFile.close();
  sqlite3_finalize(stmt);
  std::cout << "Data successfully written to output.json" << std::endl;

  return 0;
};

json SqlDatabase::FillGroup(int i, const char* name) {
  sqlite3_stmt* stmtIn;
  json row;
  json row2;
  int flagGroup = 0;
  std::string find;
  std::string sqlQuery =
      "SELECT name FROM data WHERE Name "
      "LIKE '" +
      GetPeriod(name, 0, i) + "%';";
  int rcIn = sqlite3_prepare_v2(db, sqlQuery.c_str(), -1, &stmtIn, nullptr);

  if (rcIn != SQLITE_OK) {
    throw std::logic_error("Request failed: " +
                           std::string(sqlite3_errmsg(db)));
  }
  while ((rcIn = sqlite3_step(stmtIn)) == SQLITE_ROW) {
    flagGroup = 0;
    const char* nameIn =
        reinterpret_cast<const char*>(sqlite3_column_text(stmtIn, 0));

    if (i != CountDots(nameIn)) {
      if (std::find(groupsGlobal.begin(), groupsGlobal.end(), nameIn) ==
          groupsGlobal.end()) {
        find = GetPeriod(nameIn, i, i + 1);
        row[find.erase(find.size() - 1)] = FillGroup(i + 1, nameIn);

        groupsGlobal.push_back(nameIn);
      }
    } else {
      find = GetPeriod(nameIn, i, i + 1);
      sqlQuery =
          "SELECT Value FROM data WHERE Name = '" + std::string(nameIn) + "';";
      std::string res = ExecuteQuery(sqlQuery);
      row[find] = res;
      groupsGlobal.push_back(nameIn);
      flagGroup = 1;
    }
  }
  sqlite3_finalize(stmtIn);
  return row;
};

std::string SqlDatabase::GetPeriod(const std::string& input, size_t n,
                                   size_t m) {
  int dotCount = 0;
  std::string result;

  for (char ch : input) {
    if (dotCount >= n && dotCount <= m) {
      result += ch;
    }
    if (ch == '.') {
      dotCount++;

    } else if (dotCount > 0 && dotCount < n) {
      continue;
    }

    if (dotCount >= m) {
      break;
    }
  }

  return result;
};

int SqlDatabase::CountDots(const std::string& str) {
  return std::count(str.begin(), str.end(), '.');
}

int SqlDatabase::ImportDatabase(std::string& path) {
  std::ifstream file(path);
  if (!file.is_open()) {
    throw std::logic_error("File not found");
  }
  json j;
  file >> j;

  std::string countRowsQuery = "SELECT COUNT(*) FROM data;";
  std::string countRows = ExecuteQuery(countRowsQuery);
  int rows = std::stoi(countRows);
  if (rows > 0) {
    std::cout << "A database with that name already exists, do you want to "
                 "clear it? y/n"
              << std::endl;
    std::string answer;
    std::cin >> answer;
    if (answer == "y") {
      std::string truncateQuery = "DELETE FROM data;";
      ExecuteQuery(truncateQuery);
      std::cout << "Data successfully cleared" << std::endl;
    } else {
      return 0;
    }
  }
  std::vector<std::pair<std::string, std::string>> aaa = ProcessingJson(j);
  for (auto it = aaa.begin(); it != aaa.end(); ++it) {
    WriteToDatabase(it->first, it->second);
  }
  return 0;
};

std::vector<std::pair<std::string, std::string>> SqlDatabase::ProcessingJson(
    const json& j, const std::string& group) {
  std::vector<std::pair<std::string, std::string>> result;

  for (auto it = j.begin(); it != j.end(); ++it) {
    if (it.value().is_object()) {
      auto nestedResult = ProcessingJson(it.value(), group + it.key() + ".");
      result.insert(result.end(), nestedResult.begin(), nestedResult.end());
    } else if (it.value().is_string()) {
      result.emplace_back(group + it.key(), it.value().get<std::string>());
    }
  }

  return result;
}

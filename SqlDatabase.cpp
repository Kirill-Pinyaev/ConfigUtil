#include "SqlDatabase.h"

SqlDatabase::SqlDatabase(std::string& vault) {
  int resultOpen = sqlite3_open(vault.c_str(), &db);
  if (resultOpen != SQLITE_OK) {
    throw std::logic_error("Opening or creating a database failed!");
  }
};

SqlDatabase::~SqlDatabase() { sqlite3_close(db); };

bool SqlDatabase::GroupExists(const std::string& groupName) {
  bool res = true;
  std::string sqlQuery =
      "SELECT 1 FROM data WHERE GroupName = '" + groupName + "' LIMIT 1;";
  std::string resultRequest = ExecuteQuery(sqlQuery);
  if (resultRequest.empty()) {
    res = false;
  }
  return res;
};

bool SqlDatabase::ParamExistsNonGroup(const std::string& paramName) {
  bool res;
  std::string sqlQuery =
      "SELECT 1 FROM data WHERE ParameterName = '" + paramName + "';";
  std::string resultRequest = ExecuteQuery(sqlQuery);
  sqlQuery =
      "SELECT GroupName FROM data WHERE ParameterName = '" + paramName + "';";
  std::string resultRequestGroup = ExecuteQuery(sqlQuery);
  if (!(resultRequest.empty()) && resultRequestGroup.empty()) {
    res = false;
  } else if (resultRequest.empty() && resultRequestGroup.empty()) {
    res = true;

  } else {
    throw std::logic_error("Parameter " + paramName + " is not in this group ");
  }
  return res;
}

int SqlDatabase::ParamExists(const std::string& paramName,
                             const std::string& groupName) {
  int res = 0;
  std::string sqlQuery =
      "SELECT 1 FROM data WHERE ParameterName = '" + paramName + "';";
  std::string resultRequest = ExecuteQuery(sqlQuery);
  if (!(resultRequest.empty())) {
    sqlQuery =
        "SELECT GroupName FROM data WHERE ParameterName = '" + paramName + "';";
    std::string resultRequestGroup = ExecuteQuery(sqlQuery);
    if (groupName == resultRequestGroup) {
      res = 1;
    } else {
      throw std::logic_error("Parameter " + paramName +
                             " is not in this group ");
    }
  } else {
    sqlQuery = "SELECT ParameterName FROM data WHERE GroupName = '" +
               groupName + "' LIMIT 1;";
    resultRequest = ExecuteQuery(sqlQuery);
    if (!(resultRequest.empty())) {
      res = 2;
    }
  }
  return res;
};

bool SqlDatabase::ParentPath(const std::string& group,
                             const std::string& parentGroup) {
  bool res = true;
  std::string sqlQuery =
      "SELECT ParentGroup FROM data WHERE GroupName = '" + group + "';";

  std::string resultRequest = ExecuteQuery(sqlQuery);
  if (parentGroup != resultRequest) {
    res = false;
  }
  return res;
};

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
        "GroupName TEXT,"
        "ParentGroup TEXT,"
        "ParameterName TEXT PRIMARY KEY,"
        "ParameterValue TEXT,"
        "FOREIGN KEY (ParentGroup) REFERENCES data(GroupName));";

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
  std::set<std::string> groupsDuplicate(paramGroup.begin(), paramGroup.end());
  if (groupsDuplicate.size() != paramGroup.size()) {
    throw std::logic_error("Groups should not be repeated");
  }
  std::string groupName;
  std::string parentGroup;
  std::string paramName;
  std::string exitString = "";
  if (paramGroup.size() > 1) {
    groupName = paramGroup[0];
    if (GroupExists(groupName)) {
      if (!(ParentPath(groupName, "1"))) {
        throw std::logic_error("Wrong way");
      }
    }
    for (size_t it = 1; it < paramGroup.size() - 2; ++it) {
      const auto& groupName = paramGroup[it];
      const auto& parentGroup = paramGroup[it - 1];
      if (GroupExists(groupName)) {
        if (!(ParentPath(groupName, parentGroup))) {
          throw std::logic_error("Wrong way");
        }
      }
    }
    if (paramGroup.size() == 2) {
      parentGroup = "1";
      groupName = paramGroup[0];
      paramName = paramGroup[1];
    } else {
      parentGroup = paramGroup.rbegin()[2];
      groupName = paramGroup.rbegin()[1];
      paramName = paramGroup.back();
    }
    if (GroupExists(groupName)) {
      if (!(ParentPath(groupName, parentGroup))) {
        throw std::logic_error("Wrong way");
      } else {
        std::string sqlQuery =
            "SELECT ParameterValue FROM data WHERE GroupName = '" + groupName +
            "' AND ParameterName = '" + paramName + "';";

        exitString = ExecuteQuery(sqlQuery);
      }
    }
  } else {
    paramName = paramGroup[0];
    groupName = "";
    std::string sqlQuery =
        "SELECT ParameterValue FROM data WHERE GroupName = '" + groupName +
        "' AND ParameterName = '" + paramName + "';";

    exitString = ExecuteQuery(sqlQuery);
  }
  std::cout << exitString << std::endl;
  return 0;
};

int SqlDatabase::WriteToDatabase(std::string& key, std::string& value) {
  SplitKey(key);
  std::set<std::string> groupsDuplicate(paramGroup.begin(), paramGroup.end());
  if (groupsDuplicate.size() != paramGroup.size()) {
    throw std::logic_error("Groups should not be repeated");
  }
  std::string groupName;
  std::string parentGroup;
  std::string paramName;
  if (paramGroup.size() > 1) {
    groupName = paramGroup[0];
    if (!(GroupExists(groupName))) {
      std::string sqlQueryFirst =
          "INSERT INTO data (GroupName) "
          " VALUES ('" +
          groupName + "');";
      ExecuteQuery(sqlQueryFirst);
    } else {
      if (!(ParentPath(groupName, ""))) {
        throw std::logic_error("Wrong way");
      }
    }

    for (size_t it = 1; it < paramGroup.size() - 2; ++it) {
      const auto& groupName = paramGroup[it];
      const auto& parentGroup = paramGroup[it - 1];
      if (!(GroupExists(groupName))) {
        std::string sqlQueryGroup =
            "INSERT INTO data (GroupName, ParentGroup) "
            " VALUES ('" +
            groupName + "', '" + parentGroup + "');";
        ExecuteQuery(sqlQueryGroup);
      } else {
        if (!(ParentPath(groupName, parentGroup))) {
          throw std::logic_error("Wrong way");
        }
      }
    }
    if (paramGroup.size() == 2) {
      groupName = paramGroup[0];
      paramName = paramGroup[1];
      parentGroup = "";
    } else {
      groupName = paramGroup.rbegin()[1];
      paramName = paramGroup.back();
      parentGroup = paramGroup.rbegin()[2];
    }
    if (!(GroupExists(groupName))) {
      std::string sqlQueryLast =
          "INSERT INTO data (GroupName, ParentGroup, "
          "ParameterName, "
          "ParameterValue) VALUES ('" +
          groupName + "', '" + parentGroup + "', '" + paramName + "', '" +
          value + "');";
      ExecuteQuery(sqlQueryLast);
    } else {
      if (!(ParentPath(groupName, parentGroup))) {
        throw std::logic_error("Wrong way");
      } else {
        if (ParamExists(paramName, groupName) == 1) {
          std::string sqlQueryLast =
              "UPDATE data "
              "SET ParameterValue = '" +
              value +
              "' "
              "WHERE ParameterName = '" +
              paramName + "';";
          ExecuteQuery(sqlQueryLast);
        } else if (ParamExists(paramName, groupName) == 0) {
          std::string sqlQueryLast =
              "UPDATE data "
              "SET ParameterValue = '" +
              value + "', ParameterName = '" + paramName +
              "' "
              "WHERE GroupName = '" +
              groupName + "';";
          ExecuteQuery(sqlQueryLast);
        } else {
          std::string sqlQuery =
              "INSERT OR REPLACE INTO data (GroupName, ParentGroup, "
              "ParameterName, "
              "ParameterValue) VALUES ('" +
              groupName + "', '" + parentGroup + "', '" + paramName + "', '" +
              value + "');";
          ExecuteQuery(sqlQuery);
        }
      }
    }

  } else {
    groupName = "";
    parentGroup = "";
    paramName = paramGroup[0];
    if (ParamExistsNonGroup(paramName)) {
      std::string sqlQuery =
          "INSERT OR REPLACE INTO data ("
          "ParameterName, "
          "ParameterValue) VALUES ('" +
          paramName + "', '" + value + "');";
      ExecuteQuery(sqlQuery);
    } else {
      std::string sqlQueryLast =
          "UPDATE data "
          "SET ParameterValue = '" +
          value +
          "' "
          "WHERE ParameterName = '" +
          paramName + "';";
      ExecuteQuery(sqlQueryLast);
    }
  }
  std::cout << "Recording is successful" << std::endl;
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
    const char* groupName =
        reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
    const char* parentGroup =
        reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));

    const char* paramName =
        reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
    const char* paramValue =
        reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
    if (!groupName && !parentGroup) {
      if (paramName) {
        jsonData[paramName] = paramValue;
      }
    }
    if (groupName && !parentGroup) {
      if (paramName) {
        jsonData[groupName][paramName] = paramValue;
      }
    }
  }
  std::vector<std::string> lastGroups = GetLastGroup(stmt, rc);
  for (auto& group : lastGroups) {
    std::string groupNow = group;
    std::string groupNext;
    json rowFirst;
    json rowSecond;
    int finishedGroup = 0;

    while (true) {
      while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        const char* groupName =
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        const char* parentGroup =
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));

        const char* paramName =
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        const char* paramValue =
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));

        if (groupName) {
          if (groupNow == groupName) {
            if (parentGroup) {
              groupNext = parentGroup;
            } else {
              finishedGroup = 1;
            }
            if (paramName) {
              rowFirst[paramName] = paramValue;
            }
          }
        }
      }
      if (finishedGroup == 1) {
        jsonData[groupNow] = rowFirst;
        break;
      }
      rowSecond[groupNow] = rowFirst;
      rowFirst = rowSecond;
      rowSecond.clear();
      groupNow = groupNext;
    }
  }

  if (rc != SQLITE_DONE) {
    throw std::logic_error("Request failed: " +
                           std::string(sqlite3_errmsg(db)));
  }
  std::ofstream outFile(path);
  outFile << std::setw(4) << jsonData << std::endl;
  outFile.close();

  std::cout << "Data successfully written to output.json" << std::endl;

  return 0;
};

std::vector<std::string> SqlDatabase::GetLastGroup(sqlite3_stmt* stmt, int rc) {
  std::vector<std::string> lastGroups;
  std::vector<std::string> parentGroups;
  int flag = 1;
  for (int i = 0; i < 2; i++) {
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
      const char* groupName =
          reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
      const char* parentGroup =
          reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));

      if (parentGroup && flag) {
        parentGroups.push_back(parentGroup);
      }
      if (parentGroup && !flag) {
        auto it =
            std::find(parentGroups.begin(), parentGroups.end(), groupName);
        if (it == parentGroups.end()) {
          auto itLast =
              std::find(lastGroups.begin(), lastGroups.end(), groupName);
          if (itLast == lastGroups.end()) {
            lastGroups.push_back(groupName);
          }
        }
      }
    }
    flag = 0;
  }
  return lastGroups;
};

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
  ProcessingJson(j);
  return 0;
};

void SqlDatabase::InsertData(const std::string& groupName,
                             const std::string& parentGroup,
                             const std::string& paramName,
                             const std::string& paramValue) {
  std::string insertQuery;
  if (groupName == "") {
    insertQuery =
        "INSERT INTO data (ParameterName, "
        "ParameterValue) VALUES ('" +
        paramName + "', '" + paramValue + "');";
  }
  if (parentGroup == "") {
    insertQuery =
        "INSERT INTO data (GroupName, ParameterName, "
        "ParameterValue) VALUES ('" +
        groupName + "', '" + paramName + "', '" + paramValue + "');";
  } else {
    insertQuery =
        "INSERT INTO data (GroupName, ParentGroup, ParameterName, "
        "ParameterValue) VALUES ('" +
        groupName + "', '" + parentGroup + "', '" + paramName + "', '" +
        paramValue + "');";
  }
  ExecuteQuery(insertQuery);
};

void SqlDatabase::ProcessingJson(const json& j, const std::string& group,
                                 const std::string parentGroup) {
  for (auto it = j.begin(); it != j.end(); ++it) {
    if (it.value().is_object()) {
      ProcessingJson(it.value(), it.key(), group);
    } else {
      InsertData(group, parentGroup, it.key(), it.value());
    }
  }
}
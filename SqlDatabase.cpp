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
  std::string sqlQuery = "SELECT 1 FROM data WHERE ParameterName = '" +
                         paramName + "' AND GroupName = '';";
  std::string resultRequest = ExecuteQuery(sqlQuery);
  if (resultRequest.empty()) {
    res = true;
  } else {
    res = false;
  }
  return res;
}

int SqlDatabase::ParamExists(const std::string& paramName,
                             const std::string& groupName) {
  int res = 2;

  std::string sqlQuery =
      "SELECT ParameterName FROM data WHERE GroupName = '" + groupName + "';";
  std::string resultRequest = ExecuteQuery(sqlQuery);
  sqlQuery = "SELECT 1 FROM data WHERE ParameterName = '" + paramName +
             "' AND GroupName = '" + groupName + "';";
  std::string resultRequestParam = ExecuteQuery(sqlQuery);
  if (resultRequest.empty()) {
    res = 0;
  } else if (!resultRequestParam.empty()) {
    res = 1;
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
        "ParameterName TEXT,"
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
      if (!(ParentPath(groupName, ""))) {
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
      parentGroup = "";
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
        if (ParamExists(paramName, groupName) == 0) {
          std::string sqlQueryLast =
              "UPDATE data "
              "SET ParameterValue = '" +
              value + "', ParameterName = '" + paramName +
              "' "
              "WHERE GroupName = '" +
              groupName + "';";
          ExecuteQuery(sqlQueryLast);
        } else if (ParamExists(paramName, groupName) == 1) {
          std::string sqlQueryLast =
              "UPDATE data "
              "SET ParameterValue = '" +
              value +
              "' "
              "WHERE ParameterName = '" +
              paramName + "' AND GroupName = '" + groupName + "';";
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
    paramName = paramGroup[0];
    if (ParamExistsNonGroup(paramName)) {
      std::string sqlQuery =
          "INSERT OR REPLACE INTO data (GroupName, "
          "ParameterName, "
          "ParameterValue) VALUES ('', '" +
          paramName + "', '" + value + "');";
      ExecuteQuery(sqlQuery);
    } else {
      std::string sqlQueryLast =
          "UPDATE data "
          "SET ParameterValue = '" +
          value +
          "' "
          "WHERE ParameterName = '" +
          paramName + "' AND GroupName = '';";
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
    if (*groupName == 0 && !parentGroup) {
      if (paramName) {
        jsonData[paramName] = paramValue;
      }
    }
    if (groupName && *groupName != 0 && !parentGroup) {
      if (paramName) {
        jsonData[groupName][paramName] = paramValue;
      }
    }
  }
  std::vector<std::string> lastGroups = GetLastGroup(stmt, rc);
  for (auto& group : lastGroups) {
    if (std::find(lastGroups.begin(), lastGroups.end(), group) !=
        lastGroups.end()) {
      std::string groupNow = group;
      std::string groupNext;
      json rowFirst;
      json rowSecond;
      int finishedGroup = 0;
      std::vector<std::string> groups;
      // int flagGroupIngroup = 0;
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

          if (groupName && groupName != "") {
            if (groupNow == groupName) {
              if (parentGroup) {
                groupNext = parentGroup;

              } else {
                // printf("exit: %s\n", groupNow.c_str());
                finishedGroup = 1;
              }
              if (paramName) {
                rowFirst[paramName] = paramValue;
              }
            }
          }
        }
        groups = CheckGroup(stmt, rc, groupNow);
        if (groups.size() > 0) {
          for (auto& groupInFor : groups) {
            if (groups.size() > 1) {
              std::string groupLast =
                  FindPathGroup(stmt, rc, groupInFor, lastGroups);
              rowFirst[groupInFor] = FillGroup(stmt, rc, groupLast, groupInFor);
              if (groupLast != group) {
                lastGroups.erase(std::remove(lastGroups.begin(),
                                             lastGroups.end(), groupLast),
                                 lastGroups.end());
                for (auto& group12333 : lastGroups) {
                }
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

json SqlDatabase::FillGroup(sqlite3_stmt* stmt, int rc, std::string& groupStart,
                            std::string& groupEnd) {
  std::string groupNow = groupStart;
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
      if (groupName && groupName != "") {
        if (groupNow == groupName) {
          if (parentGroup) {
            if (groupNow == groupEnd) {
              finishedGroup = 1;
            } else {
              groupNext = parentGroup;
            }

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
      break;
    }

    rowSecond[groupNow] = rowFirst;
    rowFirst = rowSecond;
    rowSecond.clear();
    groupNow = groupNext;
  }
  return rowFirst;
};

std::string SqlDatabase::FindPathGroup(sqlite3_stmt* stmt, int rc,
                                       std::string& exitGroup,
                                       std::vector<std::string>& lastGroups) {
  std::string startGroup;
  for (auto& group : lastGroups) {
    std::string groupNow = group;
    std::string groupNext;
    int finishedGroup = 0;
    while (true) {
      while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        const char* groupNameNow =
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        const char* parentGroupNow =
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        if (groupNameNow) {
          if (groupNow == groupNameNow) {
            if (parentGroupNow) {
              groupNext = parentGroupNow;
            } else {
              finishedGroup = 1;
            }
            if (exitGroup == groupNameNow) {
              startGroup = group;
            }
          }
        }
      }
      if (!startGroup.empty()) {
        break;
      }
      if (finishedGroup == 1) {
        break;
      }
      groupNow = groupNext;
    }
    if (!startGroup.empty()) {
      break;
    }
  }
  return startGroup;
}

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

std::vector<std::string> SqlDatabase::CheckGroup(sqlite3_stmt* stmt, int rc,
                                                 std::string& group) {
  std::vector<std::string> groups;
  while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
    const char* groupNameNow =
        reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
    const char* parentGroupNow =
        reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
    if (groupNameNow && parentGroupNow) {
      if (group == parentGroupNow) {
        groups.push_back(groupNameNow);
      }
    }
  }
  return groups;
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
        "INSERT INTO data (GroupName, ParameterName, "
        "ParameterValue) VALUES ('', '" +
        paramName + "', '" + paramValue + "');";
  } else if (parentGroup == "") {
    if (paramName == "" && paramValue == "") {
      insertQuery =
          "INSERT INTO data (GroupName"
          ") VALUES ('" +
          groupName + "');";
    } else {
      insertQuery =
          "INSERT INTO data (GroupName, ParameterName, "
          "ParameterValue) VALUES ('" +
          groupName + "', '" + paramName + "', '" + paramValue + "');";
    }
  } else if (paramName == "" && paramValue == "") {
    insertQuery =
        "INSERT INTO data (GroupName, ParentGroup"
        ") VALUES ('" +
        groupName + "', '" + parentGroup + "');";
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
  bool groupHasParameters = true;

  for (auto it = j.begin(); it != j.end(); ++it) {
    if (it.value().is_object()) {
      ProcessingJson(it.value(), it.key(), group);
    } else {
      InsertData(group, parentGroup, it.key(), it.value());
      groupHasParameters = false;
    }
  }

  if (groupHasParameters) {
    InsertData(group, parentGroup, "", "");
  }
}

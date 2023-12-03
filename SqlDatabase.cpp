#include "SqlDatabase.h"

SqlDatabase::SqlDatabase(std::string& vault) {
  int resultOpen = sqlite3_open(vault.c_str(), &db);
  if (resultOpen != SQLITE_OK) {
    throw std::logic_error("Opening or creating a database failed!");
  }
};

SqlDatabase::~SqlDatabase() { sqlite3_close(db); };

// int SqlDatabase::SplitKey(std::string& key) {
//   std::string part;
//   std::istringstream tokenStream(key);
//   while (std::getline(tokenStream, part, '.')) {
//     paramGroup.push_back(part);
//   }
//   return 1;
// };

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
  // sqlQuery =
  //     "SELECT name FROM data WHERE Name "
  //     "LIKE 'a.%';";
  // sqlite3_stmt* stmt;
  // int rc = sqlite3_prepare_v2(db, sqlQuery.c_str(), -1, &stmt, nullptr);

  // if (rc != SQLITE_OK) {
  //   throw std::logic_error("Request failed: " +
  //                          std::string(sqlite3_errmsg(db)));
  // }

  // while (sqlite3_step(stmt) == SQLITE_ROW) {
  //   // Получаем значение из результата и выводим его
  //   const char* value =
  //       reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
  //   std::cout << "Value: " << value << std::endl;
  // }
  // sqlite3_finalize(stmt);
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
    const char* name =
        reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
    const char* value =
        reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
    std::string nameGroup = GetSubstringUntilNPeriod(name, 0, 1);
    nameGroup = nameGroup.erase(nameGroup.size() - 1);
    if (jsonData.find(nameGroup) == jsonData.end()) {
      jsonData[nameGroup] = FillGroup(1, name);
    }
    // printf("%s\n", jsonData.dump().c_str());
  }
  //   const char* groupName =
  //       reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
  //   const char* parentGroup =
  //       reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));

  //   const char* paramName =
  //       reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
  //   const char* paramValue =
  //       reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
  //   if (*groupName == 0 && !parentGroup) {
  //     if (paramName) {
  //       jsonData[paramName] = paramValue;
  //     }
  //   }
  //   if (groupName && *groupName != 0 && !parentGroup) {
  //     if (paramName) {
  //       jsonData[groupName][paramName] = paramValue;
  //     }
  //   }
  // }
  // std::vector<std::string> lastGroups = GetLastGroup(stmt, rc);
  // for (auto& group : lastGroups) {
  //   if (std::find(lastGroups.begin(), lastGroups.end(), group) !=
  //       lastGroups.end()) {
  //     std::string groupNow = group;
  //     std::string groupNext;
  //     json rowFirst;
  //     json rowSecond;
  //     int finishedGroup = 0;
  //     std::vector<std::string> groups;
  //     // int flagGroupIngroup = 0;
  //     while (true) {
  //       while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
  //         const char* groupName =
  //             reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
  //         const char* parentGroup =
  //             reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));

  //         const char* paramName =
  //             reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
  //         const char* paramValue =
  //             reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));

  //         if (groupName && groupName != "") {
  //           if (groupNow == groupName) {
  //             if (parentGroup) {
  //               groupNext = parentGroup;

  //             } else {
  //               // printf("exit: %s\n", groupNow.c_str());
  //               finishedGroup = 1;
  //             }
  //             if (paramName) {
  //               rowFirst[paramName] = paramValue;
  //             }
  //           }
  //         }
  //       }
  //       groups = CheckGroup(stmt, rc, groupNow);
  //       if (groups.size() > 0) {
  //         for (auto& groupInFor : groups) {
  //           if (groups.size() > 1) {
  //             std::string groupLast =
  //                 FindPathGroup(stmt, rc, groupInFor, lastGroups);
  //             rowFirst[groupInFor] = FillGroup(stmt, rc, groupLast,
  //             groupInFor); if (groupLast != group) {
  //               lastGroups.erase(std::remove(lastGroups.begin(),
  //                                            lastGroups.end(), groupLast),
  //                                lastGroups.end());
  //               for (auto& group12333 : lastGroups) {
  //               }
  //             }
  //           }
  //         }
  //       }

  //       if (finishedGroup == 1) {
  //         jsonData[groupNow] = rowFirst;
  //         break;
  //       }

  //       rowSecond[groupNow] = rowFirst;
  //       rowFirst = rowSecond;
  //       rowSecond.clear();
  //       groupNow = groupNext;
  //     }
  //   }
  // }
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
  // printf("%s\n", name);
  // printf("%d\n", i);
  // printf("%s\n", find.c_str());
  // // printf("%s\n", find.c_str());
  std::string sqlQuery =
      "SELECT name FROM data WHERE Name "
      "LIKE '" +
      GetSubstringUntilNPeriod(name, 0, i) + "%';";
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
        find = GetSubstringUntilNPeriod(nameIn, i, i + 1);
        row[find.erase(find.size() - 1)] = FillGroup(i + 1, nameIn);

        groupsGlobal.push_back(nameIn);
      }
    } else {
      find = GetSubstringUntilNPeriod(nameIn, i, i + 1);
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

std::string SqlDatabase::GetSubstringUntilNPeriod(const std::string& input,
                                                  size_t n, size_t m) {
  int dotCount = 0;
  std::string result;

  for (char ch : input) {
    if (dotCount >= n && dotCount <= m) {
      result += ch;
    }
    if (ch == '.') {
      dotCount++;

    } else if (dotCount > 0 && dotCount < n) {
      // Пропускаем символы до n-й точки
      continue;
    }

    if (dotCount >= m) {
      // Достигнута m-я точка, завершаем цикл
      break;
    }
  }

  return result;
};

int SqlDatabase::CountDots(const std::string& str) {
  return std::count(str.begin(), str.end(), '.');
}

// std::string SqlDatabase::FindPathGroup(sqlite3_stmt* stmt, int rc,
//                                        std::string& exitGroup,
//                                        std::vector<std::string>& lastGroups)
//                                        {
//   std::string startGroup;
//   for (auto& group : lastGroups) {
//     std::string groupNow = group;
//     std::string groupNext;
//     int finishedGroup = 0;
//     while (true) {
//       while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
//         const char* groupNameNow =
//             reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
//         const char* parentGroupNow =
//             reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
//         if (groupNameNow) {
//           if (groupNow == groupNameNow) {
//             if (parentGroupNow) {
//               groupNext = parentGroupNow;
//             } else {
//               finishedGroup = 1;
//             }
//             if (exitGroup == groupNameNow) {
//               startGroup = group;
//             }
//           }
//         }
//       }
//       if (!startGroup.empty()) {
//         break;
//       }
//       if (finishedGroup == 1) {
//         break;
//       }
//       groupNow = groupNext;
//     }
//     if (!startGroup.empty()) {
//       break;
//     }
//   }
//   return startGroup;
// }

// std::vector<std::string> SqlDatabase::GetLastGroup(sqlite3_stmt* stmt, int
// rc) {
//   std::vector<std::string> lastGroups;
//   std::vector<std::string> parentGroups;
//   int flag = 1;
//   for (int i = 0; i < 2; i++) {
//     while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
//       const char* groupName =
//           reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
//       const char* parentGroup =
//           reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));

//       if (parentGroup && flag) {
//         parentGroups.push_back(parentGroup);
//       }
//       if (parentGroup && !flag) {
//         auto it =
//             std::find(parentGroups.begin(), parentGroups.end(), groupName);
//         if (it == parentGroups.end()) {
//           auto itLast =
//               std::find(lastGroups.begin(), lastGroups.end(), groupName);
//           if (itLast == lastGroups.end()) {
//             lastGroups.push_back(groupName);
//           }
//         }
//       }
//     }
//     flag = 0;
//   }
//   return lastGroups;
// };

// std::vector<std::string> SqlDatabase::CheckGroup(sqlite3_stmt* stmt, int rc,
//                                                  std::string& group) {
//   std::vector<std::string> groups;
//   while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
//     const char* groupNameNow =
//         reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
//     const char* parentGroupNow =
//         reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
//     if (groupNameNow && parentGroupNow) {
//       if (group == parentGroupNow) {
//         groups.push_back(groupNameNow);
//       }
//     }
//   }
//   return groups;
// }

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

void SqlDatabase::InsertData(const std::string& name,
                             const std::string& value) {
  std::string insertQuery = "INSERT INTO data (Name, Value) VALUES ('" + name +
                            "', '" + value + "');";
  ExecuteQuery(insertQuery);
};

void SqlDatabase::ProcessingJson(const json& j, const std::string& group) {
  for (auto it = j.begin(); it != j.end(); ++it) {
    if (it.value().is_object()) {
      ProcessingJson(it.value(), it.key());
    } else {
      InsertData(it.key(), it.value());
    }
  }
}

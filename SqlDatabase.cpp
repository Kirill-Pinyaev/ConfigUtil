#include "SqlDatabase.h"

SqlDatabase::SqlDatabase(std::string& vault, int flagVault) {
  std::string filePath;
  if (flagVault == 0) {
    filePath = "../config.sqlite";
  } else {
    filePath = vault;
  }
  int resultOpen = sqlite3_open(filePath.c_str(), &db);
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
    result = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
  }
  sqlite3_finalize(stmt);
  return result;
  result = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
}
int SqlDatabase::CreateDatabase() {
  std::string request =
      "SELECT name FROM sqlite_master WHERE type='table' AND name='data';";
  std::string result = ExecuteQuery(request);

  if (result.empty()) {
    const char* createTableQuery =
        "CREATE TABLE data ("
        "GroupName TEXT PRIMARY KEY,"
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
          "INSERT INTO data (GroupName, ParentGroup) "
          " VALUES ('" +
          groupName + "', '" + "1" + "');";
      ExecuteQuery(sqlQueryFirst);
    } else {
      if (!(ParentPath(groupName, "1"))) {
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
      parentGroup = "1";
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
        std::string sqlQueryLast =
            "INSERT OR REPLACE INTO data (GroupName, ParentGroup, "
            "ParameterName, "
            "ParameterValue) VALUES ('" +
            groupName + "', '" + parentGroup + "', '" + paramName + "', '" +
            value + "');";
        ExecuteQuery(sqlQueryLast);
      }
    }
  } else {
    groupName = "";
    parentGroup = "";
    paramName = paramGroup[0];
    std::string sqlQuery =
        "INSERT OR REPLACE INTO data (GroupName, ParentGroup, "
        "ParameterName, "
        "ParameterValue) VALUES ('" +
        groupName + "', '" + parentGroup + "', '" + paramName + "', '" + value +
        "');";
    ExecuteQuery(sqlQuery);
  }
  std::cout << "Recording is successful" << std::endl;
  return 0;
};
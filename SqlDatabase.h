#ifndef _SQL_DATABASE_H_
#define _SQL_DATABASE_H_

#include <fstream>
#include <iostream>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

#include "build/_deps/json-src/include/nlohmann/json.hpp"
#include "lib/sqlite/sqlite3.h"

using json = nlohmann::json;

class SqlDatabase {
 private:
  sqlite3* db;
  std::vector<std::string> paramGroup;
  std::string ExecuteQuery(std::string& query);
  bool GroupExists(const std::string& groupName,
                   const std::string& parentGroup);
  bool ParentPath(const std::string& group, const std::string& parentGroup);
  int ParamExists(const std::string& paramName, const std::string& groupName);
  bool ParamExistsNonGroup(const std::string& paramName);
  std::vector<std::string> GetLastGroup(sqlite3_stmt* stmt, int rc);
  void InsertData(const std::string& Name, const std::string& Value);
  void ProcessingJson(const json& j, const std::string& group = "");

  std::string GetSubstringUntilNPeriod(const std::string& input, size_t n,
                                       size_t m);

 public:
  SqlDatabase(std::string& vault);
  ~SqlDatabase();
  int SplitKey(std::string& key);
  int CreateDatabase();
  int ReadDatabase(std::string& key);
  int WriteToDatabase(std::string& key, std::string& value);
  int ExportDatabase(std::string& path);
  int ImportDatabase(std::string& path);
  std::vector<std::string> CheckGroup(sqlite3_stmt* stmt, int rc,
                                      std::string& group);
  json FillGroup(int i, const char* name);
  std::string FindPathGroup(sqlite3_stmt* stmt, int rc, std::string& exitGroup,
                            std::vector<std::string>& lastGroups);
};

#endif

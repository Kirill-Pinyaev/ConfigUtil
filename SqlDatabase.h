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
  std::vector<std::string> groupsGlobal;
  std::vector<std::string> paramGroup;
  std::string ExecuteQuery(std::string& query);
  std::vector<std::pair<std::string, std::string>> ProcessingJson(
      const json& j, const std::string& group = "");

  std::string GetPeriod(const std::string& input, size_t n, size_t m);
  int CountDots(const std::string& str);

 public:
  SqlDatabase(std::string& vault);
  ~SqlDatabase();
  int CreateDatabase();
  int ReadDatabase(std::string& key);
  int WriteToDatabase(std::string& key, std::string& value);
  int ExportDatabase(std::string& path);
  int ImportDatabase(std::string& path);
  json FillGroup(int i, const char* name);
};

#endif

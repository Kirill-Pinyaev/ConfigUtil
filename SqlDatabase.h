#ifndef _SQL_DATABASE_H_
#define _SQL_DATABASE_H_

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "lib/sqlite/sqlite3.h"

class SqlDatabase {
 private:
  sqlite3* db;
  std::vector<std::string> paramGroup;
  std::string ExecuteQuery(std::string& query);
  int CallbackFunction(void* data, int argc, char** argv, char**);

 public:
  SqlDatabase(std::string& vault);
  ~SqlDatabase();
  int SplitKey(std::string& key);
  int CreateDatabase();
  int ReadDatabase(std::string& key);
  int WriteToDatabase(std::string& key, std::string& value);
};

#endif

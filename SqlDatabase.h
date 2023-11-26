#ifndef _SQL_DATABASE_H_
#define _SQL_DATABASE_H_

#include <iostream>
#include <stdexcept>

#include "lib/sqlite/sqlite3.h"

class SqlDatabase {
 private:
  sqlite3* db;
  std::string ExecuteQuery(std::string& query);
  int CallbackFunction(void* data, int argc, char** argv, char**);

 public:
  SqlDatabase(std::string& vault);
  int CreateDatabase();
  int ReadDatabase(std::string& key);
  int WriteToDatabase(std::string& key, std::string& value);
};

#endif

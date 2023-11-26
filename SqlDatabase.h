#ifndef _SQL_DATABASE_H_
#define _SQL_DATABASE_H_

#include <iostream>
#include <stdexcept>

#include "lib/sqlite/sqlite3.h"

class SqlDatabase {
 private:
  sqlite3* db;

 public:
  SqlDatabase(std::string& vault);
  int CreateDatabase();
};

#endif

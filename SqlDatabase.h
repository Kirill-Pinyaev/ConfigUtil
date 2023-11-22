#ifndef _SQL_DATABASE_H_
#define _SQL_DATABASE_H_

#include <iostream>
#include <stdexcept>

#include "lib/sqlite/sqlite3.h"

class SqlDatabase {
 public:
  sqlite3* db;
  SqlDatabase(std::string vault);
};

#endif

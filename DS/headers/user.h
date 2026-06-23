#pragma once
#include "sqlite3.h"
#include <string>
using namespace std;
class User {
  int user_id;
  string username;
  string created_at;

public:
  User();
  void create_user(int user_id, string username);
  void save_user(sqlite3 *db);
};

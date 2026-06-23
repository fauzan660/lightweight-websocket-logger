#include "DS/headers/user.h"
#include "sqlite3.h"
#include <iostream>
#include <string>
using namespace std;

User::User() {
  this->user_id = 0;
  this->username = "";
  this->created_at = "";
}
void User::create_user(int user_id, string username) {
  this->user_id = user_id;
  this->username = username;
  time_t now = time(nullptr);
  this->created_at = ctime(&now);
}
void User::save_user(sqlite3 *db) {
  string sql = "INSERT INTO Users (username, created_at) VALUES ('" +
               this->username + "', '" + this->created_at + "');";

  char *err;
  if (sqlite3_exec(db, sql.c_str(), NULL, NULL, &err) != SQLITE_OK) {
    fprintf(stderr, "SQL error: %s\n", err);
    sqlite3_free(err);
  }
}

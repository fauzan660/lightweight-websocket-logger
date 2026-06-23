#include "../headers/db.h"
#include <iostream>
using namespace std;

sqlite3 *init_db() {
  sqlite3 *db;
  string db_path = "/Users/fauzantahir/Documents/personal/tcp_chat_server/c++/"
                   "non_block/database/debugrooms.db";
  int exit = 0;
  int create;
  char *messageError;
  exit = sqlite3_open(db_path.c_str(), &db);

  if (exit) {
    cerr << "[ERROR]: " << sqlite3_errmsg(db) << endl;
  } else
    cout << "Opened db successfully" << endl;
  create = sqlite3_exec(db,
                        "CREATE TABLE IF NOT EXISTS Rooms ("
                        "room_id INTEGER PRIMARY KEY AUTOINCREMENT,"
                        "title TEXT,"
                        "creator_id INTEGER,"
                        "status TEXT DEFAULT 'open'"
                        ");"

                        "CREATE TABLE IF NOT EXISTS Users ("
                        "user_id INTEGER PRIMARY KEY AUTOINCREMENT,"
                        "username TEXT,"
                        "created_at TEXT"
                        ");"

                        "CREATE TABLE IF NOT EXISTS Suggestions ("
                        "suggestion_id INTEGER PRIMARY KEY AUTOINCREMENT,"
                        "room_id INTEGER,"
                        "author_id INTEGER,"
                        "content TEXT,"
                        "created_at TEXT,"
                        "is_accepted INTEGER DEFAULT 0"
                        ");"

                        "CREATE TABLE IF NOT EXISTS Code ("
                        "code_id INTEGER PRIMARY KEY AUTOINCREMENT,"
                        "room_id INTEGER UNIQUE,"
                        "code_snippet TEXT,"
                        "error_message TEXT,"
                        "language TEXT,"
                        "context TEXT"
                        ");",

                        NULL, NULL, &messageError);
  if (create != SQLITE_OK) {
    cerr << "Error Create " << endl;
    sqlite3_free(messageError);
  } else
    cout << "Records created Successfully!" << endl;
  sqlite3_exec(
      db,
      "INSERT OR IGNORE INTO sqlite_sequence (name, seq) VALUES "
      "('Users', 99), ('Rooms', 199), ('Code', 299), ('Suggestions', 399);",
      NULL, NULL, NULL);
  return db;
}
int callback(void *data, int argc, char **argv, char **azColName) {
  int i;
  fprintf(stderr, "%s: \n", (const char *)data);

  for (i = 0; i < argc; i++) {
    printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
  }

  printf("\n");
  return 0;
}

#include "../headers/action_handler.h"
#include "../headers/db.h"
#include "DS/headers/code.h"
#include "DS/headers/room.h"
#include "DS/headers/suggestion.h"
#include "DS/headers/user.h"
#include "headers/db.h"
#include "sqlite3.h"
#include <iostream>
#include <nlohmann/json.hpp>
using namespace std;
using json = nlohmann::json;

void create_user(json data) {
  sqlite3 *db = init_db();

  json payload = data["payload"];
  int user_id = payload["user_id"];
  string username = payload["username"];

  User *user = new User();
  user->create_user(user_id, username);
  user->save_user(db);

  string query = "SELECT * FROM Users;";

  sqlite3_exec(db, query.c_str(), callback, NULL, NULL);
}
void create_room(json data) {}
void view_room(json data) {}
void add_suggestion(json data) {}

void handle_action(Action action, json json_request) {
  switch (action) {
  case Action::CREATE_USER:
    create_user(json_request);
    break;
  case Action::CREATE_ROOM:
    create_room(json_request);
    break;
  case Action::VIEW_ROOM:
    view_room(json_request);
    break;
  case Action::ADD_SUGGESTION:
    add_suggestion(json_request);
    break;
  case Action::UNKNOWN:
    cout << "[ERROR]The action entered is unknown" << endl;
    break;
  }
}

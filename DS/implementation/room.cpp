#include "../headers/room.h"
#include <ctime>
#include <iostream>
using namespace std;

Room::Room(int room_id, string title, int creator_id) {
  this->room_id = room_id;
  this->title = title;
  this->status = "open";
  time_t now = time(nullptr);
  this->created_at = ctime(&now);
  this->creator_id = creator_id;
  this->content = nullptr;
}

void Room::add_content(string code_snippet, string error_message,
                       string language, string context) {
  this->content = new Code();
  this->content->add(code_snippet, error_message, language, context);
}

#include "../headers/suggestion.h"
#include <string>
using namespace std;
Suggestion::Suggestion(int suggestion_id, int room_id, int author_id) {
  this->suggestion_id = suggestion_id;
  this->room_id = room_id;
  this->author_id = author_id;
  this->content = "";
  time_t now = time(nullptr);
  this->created_at = ctime(&now);
  this->is_accepted = false;
}
void Suggestion::add_suggestion(string content) { this->content = content; }

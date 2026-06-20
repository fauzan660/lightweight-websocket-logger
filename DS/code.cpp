#include "code.h"
Code::Code() {
  this->code_snippet = "";
  this->error_message = "";
  this->context = "";
  this->language = "";
}
void Code::add(string code_snippet, string error_message, string language,
               string context) {
  this->code_snippet = code_snippet;
  this->error_message = error_message;
  this->language = language;
  this->context = context;
}

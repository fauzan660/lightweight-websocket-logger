#pragma once
#include "../headers/action_handler.h"
#include <iostream>
#include <nlohmann/json.hpp>

using namespace std;
using json = nlohmann::json;

string encode_key_openssl(string key);
string encode_key_base64(string &key);
string generate_websocket_response_key(string websocket_request_key);
json parse_websocket_request(uint8_t websocket_buffer[], int request_size);
Action get_client_action(json json_object);

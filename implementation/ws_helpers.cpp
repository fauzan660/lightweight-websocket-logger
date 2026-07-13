#include "../headers/ws_helpers.h"
#include "../headers/action_handler.h"
#include "WS_Frame/headers/frame.h"
#include <iostream>
#include <nlohmann/json.hpp>
#include <openssl/evp.h>
#include <openssl/sha.h>
#define MAGIC_WEBSOCKET_UUID_STRING "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"

using namespace std;
string encode_key_openssl(string key) {
  unsigned char hash[SHA_DIGEST_LENGTH];
  SHA1((unsigned char *)key.c_str(), key.size(), hash);
  return string(hash, hash + SHA_DIGEST_LENGTH);
}

string encode_key_base64(string &key) {
  string output(((key.size() + 2) / 3) * 4, '\0');
  EVP_EncodeBlock((unsigned char *)output.data(), (unsigned char *)key.c_str(),
                  key.size());
  return output;
}

string generate_websocket_response_key(string websocket_request_key) {
  string concat = websocket_request_key + MAGIC_WEBSOCKET_UUID_STRING;
  string hashed = encode_key_openssl(concat);
  if (hashed.empty())
    return "";
  string encoded = encode_key_base64(hashed);
  if (encoded.empty())
    return "";
  return encoded;
}
void parse_websocket_request(uint8_t websocket_buffer[], int request_size,
                             const string direction) {
  WS_Frame frame;
  frame.parse(websocket_buffer, request_size);
  printf("%.*s\n", (int)frame.get_payload_data().size(),
         frame.get_payload_data().data());

  vector<uint8_t> decoded_req = frame.get_payload_data();
  string data_recv(reinterpret_cast<char *>(decoded_req.data()),
                   decoded_req.size());
  string opcode_str;
  switch (frame.get_opcode()) {
  case 0x1:
    opcode_str = "text";
    break;
  case 0x2:
    opcode_str = "binary";
    break;
  case 0x9:
    opcode_str = "ping";
    break;
  case 0xA:
    opcode_str = "pong";
    break;
  default:
    opcode_str = "unknown";
    break;
  }

  printf("[%s] %s | %.*s\n", direction.c_str(), opcode_str.c_str(),
         (int)decoded_req.size(), decoded_req.data());
}
Action get_client_action(json json_object) {
  string data = json_object["action"];
  if (data == "CREATE_USER")
    return Action::CREATE_USER;
  if (data == "CREATE_ROOM")
    return Action::CREATE_ROOM;
  if (data == "VIEW_ROOM_SUGGESTIONS")
    return Action::VIEW_ROOM_SUGGESTIONS;
  if (data == "ADD_SUGGESTION")
    return Action::ADD_SUGGESTION;
  if (data == "ACCEPT_SUGGESTION")
    return Action::ACCEPT_SUGGESTION;

  return Action::UNKNOWN;
}

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <errno.h>
#include <fcntl.h>
#include <iostream>
#include <map>
#include <netinet/in.h>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
using namespace std;
#define MAX_CONNECTIONS 5
#define MAX_CLIENTS 10
#define BUF_SIZE 4096
#define MAGIC_WEBSOCKET_UUID_STRING "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"

typedef struct {
  int fd;
} Client;

void close_socket(Client *c) {
  ::close(c->fd);
  c->fd = -1;
}

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

bool check_request_validity(string method, string target, string http_version,
                            map<string, string> headers_map) {

  if (method == "GET") {
    string http_version_number =
        http_version.substr(http_version.find('/') + 1, http_version.length());
    int http_vn = stoi(http_version_number);
    if (http_vn >= 1.1) {
      auto upg = headers_map.find("upgrade");
      if (upg != headers_map.end() && upg->second == "websocket") {
        auto con = headers_map.find("connection");
        if (con != headers_map.end() && con->second == "Upgrade") {
          auto key = headers_map.find("sec-websocket-key");
          if (key != headers_map.end()) {
            return true;
          }
        }
      }
    }
  }
  return false;
}

string generate_websocket_response_key(string websocket_request_key) {
  string magic_string_concat =
      websocket_request_key + MAGIC_WEBSOCKET_UUID_STRING;
  string hashed_string = encode_key_openssl(magic_string_concat);
  string encoded_string = encode_key_base64(hashed_string);
  return encoded_string;
}

void parse_request(string message, string &method, string &target,
                   string &http_version, map<string, string> &headers_map) {
  string head = message.substr(0, message.find("\r\n\r\n"));
  string first_line = head.substr(0, head.find("\r\n"));
  size_t s1 = first_line.find(' ');
  size_t s2 = first_line.find(' ', s1 + 1);
  method = first_line.substr(0, s1);
  target = first_line.substr(s1 + 1, s2 - s1 - 1);
  http_version = first_line.substr(s2 + 1);
  string headers_str = head.substr(head.find("\r\n") + 2);
  size_t pos = 0;
  while ((pos = headers_str.find("\r\n")) != string::npos) {
    string line = headers_str.substr(0, pos);
    size_t colon = line.find(": ");
    if (colon != string::npos) {
      string key = line.substr(0, colon);
      string val = line.substr(colon + 2);
      for (auto &ch : key)
        ch = tolower(ch);
      headers_map[key] = val;
    }
    headers_str = headers_str.substr(pos + 2);
  }
}

int setup_server_socket(int &s, struct sockaddr_in &address, int PORT) {
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(PORT);
  cout << "hello" << endl;
  if ((s = ::socket(AF_INET, SOCK_STREAM, 0)) == 0) {
    perror("timeout while socket creation");
    exit(-1);
  }
  int opt = 1;
  setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
  if (::bind(s, (struct sockaddr *)&address, sizeof(address)) < 0) {
    perror("socket with fd not binded correctly to address");
    exit(-1);
  }
  if (fcntl(s, F_SETFL, O_NONBLOCK) < 0) {
    perror("making socket non blocking failed!!");
    exit(-1);
  }
  if (::listen(s, MAX_CONNECTIONS) < 0) {
    perror("socket is listening at port");
    exit(-1);
  }
  return 1;
}

void accept_clients(int s, Client clients[], fd_set &read_sockets,
                    struct sockaddr_in &address, int &address_len) {
  cout << "-------Accepting client connections------" << endl;
  while (true) {
    int client =
        accept(s, (struct sockaddr *)&address, (socklen_t *)&address_len);
    if (client < 0) {
      printf("no more client left exiting.....\n");
      break;
    }
    if (fcntl(client, F_SETFL, O_NONBLOCK) < 0)
      perror("Error making client socket non blocking");
    FD_SET(client, &read_sockets);
    cout << "client " << client << " admitted to readable sockets set" << endl;
    for (int i = 0; i < MAX_CLIENTS; i++) {
      if (clients[i].fd == -1) {
        clients[i].fd = client;
        break;
      }
    }
  }
}

void handle_client(Client *c, char server_buf[], char response_buf[],
                   fd_set &read_sockets) {
  cout << "-------Handling client connections-------" << endl;
  string message = "";
  while (true) {
    int recv_status = ::recv(c->fd, server_buf, BUF_SIZE - 1, 0);
    if (recv_status == 0) {
      printf("server waiting timeout\n");
      close_socket(c);
      break;
    }
    if (recv_status < 0) {
      if (errno == EAGAIN || errno == EWOULDBLOCK)
        break;
      printf("client closed connection\n");
      close_socket(c);
      break;
    }
    server_buf[recv_status] = '\0';
    message += string(server_buf, recv_status);
    if (message.size() > 4 && message.substr(message.size() - 4) == "\r\n\r\n")
      break;
  }
  if (c->fd == -1 || message.empty())
    return;
  cout << "Received message:\n" << message << endl;
  string method, target, http_version;
  map<string, string> headers_map;
  parse_request(message, method, target, http_version, headers_map);
  cout << "method, target, http_version: " << method << " " << target << " "
       << http_version << endl;
  cout << "headers:" << endl;
  for (auto &h : headers_map)
    cout << h.first << ": " << h.second << endl;
  ::send(c->fd, response_buf, strlen(response_buf), 0);
  close_socket(c);
}

int main() {
  int s;
  const int PORT = 8081;
  struct sockaddr_in address;
  fd_set write_sockets, read_sockets;
  struct timeval waitd;
  Client clients[MAX_CLIENTS];
  char server_buf[BUF_SIZE];
  char response_buf[] = "HTTP/1.1 200 OK\r\n"
                        "Content-Type: text/html; charset=utf-8\r\n"
                        "\r\n"
                        "<HTML><HEAD>"
                        "<meta http-equiv=\"content-type\" "
                        "content=\"text/html;charset=utf-8\">\r\n"
                        "<TITLE>200 OK</TITLE></HEAD><BODY>\r\n"
                        "<H1>200 OK</H1>\r\n"
                        "Welcome to the default.\r\n"
                        "</BODY></HTML>\r\n";

  if (setup_server_socket(s, address, PORT))
    printf("Server socket successfully setup\n");

  int max_fd = s;
  int address_len = sizeof(address);
  cout << "listening on port " << PORT << endl;
  FD_ZERO(&write_sockets);
  FD_ZERO(&read_sockets);
  FD_SET(s, &read_sockets);
  waitd.tv_sec = 10;

  for (int i = 0; i < MAX_CLIENTS; i++)
    clients[i].fd = -1;

  while (1) {
    max_fd = s;
    FD_ZERO(&read_sockets);
    FD_SET(s, &read_sockets);
    for (int i = 0; i < MAX_CLIENTS; i++) {
      Client *c = &clients[i];
      if (c->fd != -1) {
        FD_SET(c->fd, &read_sockets);
        max_fd = c->fd;
      }
    }
    int sel = ::select(max_fd + 1, &read_sockets, &write_sockets, (fd_set *)0,
                       &waitd);
    if (sel < 0)
      continue;

    if (FD_ISSET(s, &read_sockets))
      accept_clients(s, clients, read_sockets, address, address_len);

    for (int i = 0; i < MAX_CLIENTS; i++) {
      Client *c = &clients[i];
      if (c->fd == -1 || c->fd == s)
        continue;
      if (FD_ISSET(c->fd, &read_sockets))
        handle_client(c, server_buf, response_buf, read_sockets);
    }
  }
  return 0;
}

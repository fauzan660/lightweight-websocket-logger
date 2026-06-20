#include "common.h"
#include "http.h"
#include "ws.h"
#include <algorithm>
#include <cstdio>
#include <fcntl.h>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
using namespace std;

void close_socket(Client *c) {
  ::close(c->fd);
  c->fd = -1;
}

int setup_server_socket(int &s, struct sockaddr_in &address, int PORT) {
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(PORT);
  if ((s = ::socket(AF_INET, SOCK_STREAM, 0)) == 0) {
    perror("socket");
    exit(-1);
  }
  int opt = 1;
  setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
  if (::bind(s, (struct sockaddr *)&address, sizeof(address)) < 0) {
    perror("bind");
    exit(-1);
  }
  if (fcntl(s, F_SETFL, O_NONBLOCK) < 0) {
    perror("fcntl");
    exit(-1);
  }
  if (::listen(s, MAX_CONNECTIONS) < 0) {
    perror("listen");
    exit(-1);
  }
  return 1;
}

void accept_clients(int s, Client clients[], fd_set &read_sockets,
                    struct sockaddr_in &address, int &address_len) {
  while (true) {
    int client =
        accept(s, (struct sockaddr *)&address, (socklen_t *)&address_len);
    if (client < 0)
      break;
    if (fcntl(client, F_SETFL, O_NONBLOCK) < 0)
      perror("fcntl client");
    FD_SET(client, &read_sockets);
    for (int i = 0; i < MAX_CLIENTS; i++) {
      if (clients[i].fd == -1) {
        clients[i].fd = client;
        break;
      }
    }
  }
}

int main() {
  int s;
  const int PORT = 8081;
  struct sockaddr_in address;
  fd_set write_sockets, read_sockets;
  struct timeval waitd;
  Client clients[MAX_CLIENTS];
  Client websocket_clients[MAX_WS_CLIENTS];
  char server_buf[BUF_SIZE];

  const char *body = "<HTML><HEAD><TITLE>200 OK</TITLE></HEAD><BODY><H1>200 "
                     "OK</H1>Welcome.</BODY></HTML>";
  char response_buf[1024];
  snprintf(response_buf, sizeof(response_buf),
           "HTTP/1.1 200 OK\r\nContent-Type: text/html; "
           "charset=utf-8\r\nContent-Length: %zu\r\n\r\n%s",
           strlen(body), body);
  char ws_handshake_error_buf[] = "HTTP/1.1 400 Bad Request";

  setup_server_socket(s, address, PORT);
  printf("Server listening on port %d\n", PORT);

  int address_len = sizeof(address);
  FD_ZERO(&write_sockets);
  FD_ZERO(&read_sockets);
  FD_SET(s, &read_sockets);

  for (int i = 0; i < MAX_CLIENTS; i++)
    clients[i].fd = -1;
  for (int i = 0; i < MAX_WS_CLIENTS; i++)
    websocket_clients[i].fd = -1;

  while (1) {
    waitd.tv_sec = 10;
    waitd.tv_usec = 0; // reset every iteration or it decays to 0
    int max_fd = s;
    FD_ZERO(&read_sockets);
    FD_SET(s, &read_sockets);
    for (int i = 0; i < MAX_CLIENTS; i++) {
      if (clients[i].fd != -1) {
        FD_SET(clients[i].fd, &read_sockets);
        max_fd = max(max_fd, clients[i].fd);
      }
    }
    for (int i = 0; i < MAX_WS_CLIENTS; i++) {
      if (websocket_clients[i].fd != -1) {
        FD_SET(websocket_clients[i].fd, &read_sockets);
        max_fd = max(max_fd, websocket_clients[i].fd);
      }
    }

    int sel = ::select(max_fd + 1, &read_sockets, NULL, NULL, &waitd);
    if (sel < 0)
      continue;

    if (FD_ISSET(s, &read_sockets))
      accept_clients(s, clients, read_sockets, address, address_len);

    for (int i = 0; i < MAX_WS_CLIENTS; i++) {
      Client *ws_c = &websocket_clients[i];
      if (ws_c->fd != -1 && FD_ISSET(ws_c->fd, &read_sockets))
        handle_websocket_client(ws_c, clients, websocket_clients);
    }

    for (int i = 0; i < MAX_CLIENTS; i++) {
      Client *c = &clients[i];
      if (c->fd == -1 || c->fd == s)
        continue;
      if (FD_ISSET(c->fd, &read_sockets))
        handle_client(c, server_buf, response_buf, ws_handshake_error_buf,
                      read_sockets, websocket_clients, clients);
    }
  }
  return 0;
}

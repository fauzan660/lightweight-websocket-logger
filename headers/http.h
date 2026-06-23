#pragma once
#include "common.h"
#include <map>
#include <string>
#include <sys/select.h>

void parse_request(std::string message, std::string &method,
                   std::string &target, std::string &http_version,
                   std::map<std::string, std::string> &headers_map);

void handle_client(Client *c, char server_buf[], char response_buf[],
                   char error_buf[], fd_set &read_sockets,
                   Client websocket_clients[], Client clients[]);

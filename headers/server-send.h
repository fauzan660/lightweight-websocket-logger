#pragma once
#include <cstdint>

void forward_to_server(int server_fd, uint8_t *buf, int size);

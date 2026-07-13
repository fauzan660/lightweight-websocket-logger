#include "headers/server-send.h"
#include <sys/socket.h>
using namespace std;
void forward_to_server(int server_fd, uint8_t *buf, int size) {
  int status = send(server_fd, buf, size, 0);
  if (status <= 0) {
    perror("forward to server failed");
  }
}

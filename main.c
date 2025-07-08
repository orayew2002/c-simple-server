#include "pkg/router/simple_router.h"
#include <arpa/inet.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

void getUsersHandler(int fd) {
  const char *body = "Hello from users\n";

  char res[1024];
  snprintf(res, sizeof(res),
           "HTTP/1.1 200 OK\r\n"
           "Content-Type: text/plain\r\n"
           "Content-Length: %zu\r\n"
           "\r\n"
           "%s",
           strlen(body), body);

  send(fd, res, strlen(res), 0);
}

void getBooksHandler(int fd) {
  const char *body = "Hello from books\n";

  char res[1024];
  snprintf(res, sizeof(res),
           "HTTP/1.1 200 OK\r\n"
           "Content-Type: text/plain\r\n"
           "Content-Length: %zu\r\n"
           "\r\n"
           "%s",
           strlen(body), body);

  send(fd, res, strlen(res), 0);
}

int main() {
  struct Router app = RouterDefault();

  app.mGet(&app, "/users", getUsersHandler);
  app.mGet(&app, "/books", getBooksHandler);

  app.run(&app);
  return 0;
}

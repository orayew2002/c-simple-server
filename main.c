#include <arpa/inet.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define MAX_ROUTES 32
#define MAX_METHOD_LEN 8
#define MAX_PATH_LEN 128

typedef void (*HandlerFunc)(int client_fd);

struct Route {
  char method[MAX_METHOD_LEN];
  char path[MAX_PATH_LEN];
  HandlerFunc handlerFunc;
};

struct Router {
  int port;
  int routesCount;
  struct Route routes[MAX_ROUTES];

  void (*run)(struct Router *self);
  void (*get)(struct Router *self, char *path, HandlerFunc handlerFunc);
  void (*post)(struct Router *self, char *path, HandlerFunc handlerFunc);
  void (*delete)(struct Router *self, char *path, HandlerFunc handlerFunc);
};

void newMethod(struct Router *self, char *path, const char *method,
               HandlerFunc handlerFunc) {

  if (self->routesCount >= MAX_ROUTES) {
    printf("Too many routes!\n");
    return;
  }

  struct Route *r = &self->routes[self->routesCount++];

  strncpy(r->method, method, MAX_METHOD_LEN - 1);
  r->method[MAX_METHOD_LEN - 1] = '\0';

  strncpy(r->path, path, MAX_PATH_LEN - 1);
  r->path[MAX_PATH_LEN - 1] = '\0';

  r->handlerFunc = handlerFunc;
}

void Get(struct Router *self, char *path, HandlerFunc handlerFunc) {
  newMethod(self, path, "GET", handlerFunc);
}

void Post(struct Router *self, char *path, HandlerFunc handlerFunc) {
  newMethod(self, path, "POST", handlerFunc);
}

void Delete(struct Router *self, char *path, HandlerFunc handlerFunc) {
  newMethod(self, path, "DELETE", handlerFunc);
}

void Run(struct Router *self) {
  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd < 0) {
    perror("socket failed");
    return;
  }

  struct sockaddr_in addr = {0};
  addr.sin_family = AF_INET;
  addr.sin_port = htons(self->port);
  addr.sin_addr.s_addr = INADDR_ANY;

  if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    perror("bind failed");
    close(server_fd);
    return;
  }

  if (listen(server_fd, 5) < 0) {
    perror("listen failed");
    close(server_fd);
    return;
  }

  printf("Server listening on port %d\n", self->port);

  while (1) {
    int client_fd = accept(server_fd, NULL, NULL);
    if (client_fd < 0) {
      perror("accept failed");
      continue;
    }

    char buffer[1 << 10];
    ssize_t bytes_read = read(client_fd, buffer, sizeof(buffer) - 1);
    buffer[bytes_read] = '\0';

    char method[8];
    char path[128];

    sscanf(buffer, "%7s %127s", method, path);

    for (int i = 0; i <= self->routesCount; i++) {
      if (strcmp(self->routes[i].method, method) == 0 &&
          strcmp(self->routes[i].path, path) == 0) {

        self->routes[i].handlerFunc(client_fd);

        close(client_fd);
        continue;
      }
    }

    close(client_fd);
  }

  close(server_fd);
}

void getUsersHandler(int fd) {
  const char *body = "{\"message\": \"Hello from users!\"}";
  char response[256];

  snprintf(response, sizeof(response),
           "HTTP/1.1 200 OK\r\n"
           "Content-Type: application/json\r\n"
           "Content-Length: %lu\r\n"
           "\r\n"
           "%s",
           strlen(body), body);

  send(fd, response, strlen(response), 0);
}

void getBooksHandler(int fd) {
  const char *body = "{\"message\": \"Hello from books!\"}";
  char response[256];

  snprintf(response, sizeof(response),
           "HTTP/1.1 200 OK\r\n"
           "Content-Type: application/json\r\n"
           "Content-Length: %lu\r\n"
           "\r\n"
           "%s",
           strlen(body), body);

  send(fd, response, strlen(response), 0);
}

void getSongsHandler(int fd) {
  const char *body = "{\"message\": \"Hello from songs!\"}";
  char response[256];

  snprintf(response, sizeof(response),
           "HTTP/1.1 200 OK\r\n"
           "Content-Type: application/json\r\n"
           "Content-Length: %lu\r\n"
           "\r\n"
           "%s",
           strlen(body), body);

  send(fd, response, strlen(response), 0);
}

struct Router RouterDefault() {
  struct Router r;
  r.port = 8080;
  r.routesCount = 0;

  r.run = Run;
  r.get = Get;
  r.post = Post;
  r.delete = Delete;

  return r;
}

int main() {
  struct Router app = RouterDefault();

  app.get(&app, "/users", getUsersHandler);
  app.get(&app, "/books", getBooksHandler);
  app.get(&app, "/songs", getSongsHandler);

  app.run(&app);

  return 0;
}

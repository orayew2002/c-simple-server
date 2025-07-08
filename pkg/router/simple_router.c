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
  void (*mGet)(struct Router *self, const char *path, HandlerFunc handlerFunc);
  void (*mPost)(struct Router *self, const char *path, HandlerFunc handlerFunc);
  void (*mDelt)(struct Router *self, const char *path, HandlerFunc handlerFunc);
};

void newMethod(struct Router *self, const char *path, const char *method,
               HandlerFunc handlerFunc) {
  if (self->routesCount >= MAX_ROUTES) {
    fprintf(stderr, "Too many routes!\n");
    return;
  }

  struct Route *r = &self->routes[self->routesCount++];

  strncpy(r->method, method, MAX_METHOD_LEN - 1);
  r->method[MAX_METHOD_LEN - 1] = '\0';

  strncpy(r->path, path, MAX_PATH_LEN - 1);
  r->path[MAX_PATH_LEN - 1] = '\0';

  r->handlerFunc = handlerFunc;
}

void Get(struct Router *self, const char *path, HandlerFunc handlerFunc) {
  newMethod(self, path, "GET", handlerFunc);
}

void Post(struct Router *self, const char *path, HandlerFunc handlerFunc) {
  newMethod(self, path, "POST", handlerFunc);
}

void Delete(struct Router *self, const char *path, HandlerFunc handlerFunc) {
  newMethod(self, path, "DELETE", handlerFunc);
}

void send_404(int client_fd) {
  const char *response = "HTTP/1.1 404 Not Found\r\n"
                         "Content-Length: 0\r\n"
                         "Connection: close\r\n"
                         "\r\n";
  send(client_fd, response, strlen(response), 0);
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
    if (bytes_read <= 0) {
      close(client_fd);
      continue;
    }
    buffer[bytes_read] = '\0';

    char method[MAX_METHOD_LEN];
    char path[MAX_PATH_LEN];

    if (sscanf(buffer, "%7s %127s", method, path) != 2) {
      send_404(client_fd);
      close(client_fd);
      continue;
    }

    int handled = 0;
    for (int i = 0; i < self->routesCount; i++) {
      if (strcmp(self->routes[i].method, method) == 0 &&
          strcmp(self->routes[i].path, path) == 0) {

        self->routes[i].handlerFunc(client_fd);
        close(client_fd);
        handled = 1;
        break;
      }
    }

    if (!handled) {
      send_404(client_fd);
      close(client_fd);
    }
  }

  close(server_fd);
}

struct Router RouterDefault() {
  struct Router r;
  r.port = 8080;
  r.routesCount = 0;

  r.run = Run;
  r.mGet = Get;
  r.mPost = Post;
  r.mDelt = Delete;

  return r;
}

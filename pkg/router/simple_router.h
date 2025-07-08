#ifndef SIMPLE_ROUTER_H
#define SIMPLE_ROUTER_H

#define MAX_ROUTES 32

typedef void (*HandlerFunc)(int);

struct Route {
  char method[8];
  char path[128];
  HandlerFunc handlerFunc;
};

struct Router {
  int port;
  int routesCount;
  struct Route routes[MAX_ROUTES];

  void (*run)(struct Router *self);
  void (*mGet)(struct Router *self, char *path, HandlerFunc handlerFunc);
  void (*mPost)(struct Router *self, char *path, HandlerFunc handlerFunc);
  void (*mDelt)(struct Router *self, char *path, HandlerFunc handlerFunc);
};

struct Router RouterDefault();

#endif

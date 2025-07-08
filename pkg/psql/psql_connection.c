#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int prepareConnectionCNF(char *buffer, size_t buffer_size) {
  const char *user = "user";
  const char *user_val = "media";

  const char *database = "database";
  const char *db_val = "media";

  int offset = 0;
  if (buffer_size < 512)
    return -1;

  offset += 4;

  uint32_t protocol_version = htonl(196608);
  memcpy(buffer + offset, &protocol_version, 4);
  offset += 4;

  memcpy(buffer + offset, user, strlen(user));
  offset += strlen(user);
  buffer[offset++] = '\0';

  memcpy(buffer + offset, user_val, strlen(user_val));
  offset += strlen(user_val);
  buffer[offset++] = '\0';

  memcpy(buffer + offset, database, strlen(database));
  offset += strlen(database);
  buffer[offset++] = '\0';

  memcpy(buffer + offset, db_val, strlen(db_val));
  offset += strlen(db_val);
  buffer[offset++] = '\0';

  buffer[offset++] = '\0';

  uint32_t total_len = htonl(offset);
  memcpy(buffer, &total_len, 4);

  return offset;
}

int main() {
  int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_fd < 0) {
    perror("socket");
    return 1;
  }

  struct sockaddr_in addr = {0};
  addr.sin_family = AF_INET;
  addr.sin_port = htons(5432);

  if (inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr) <= 0) {
    perror("inet_pton");
    return 1;
  }

  int ct = connect(socket_fd, (struct sockaddr *)&addr, sizeof(addr));
  if (ct < 0) {
    perror("failed connecting to psql");
    return 1;
  }

  char buffer[512];
  int len = prepareConnectionCNF(buffer, sizeof(buffer));
  if (len < 0) {
    perror("failed prepare psql config");
    return -1;
  }

  ssize_t sendBytes = send(socket_fd, buffer, len, 0);
  if (sendBytes < 0) {
    perror("error when sending request");
    return -1;
  }

  char resBuffer[1024];
  ssize_t resByteslen = recv(socket_fd, resBuffer, sizeof(resBuffer), 0);
  if (resByteslen < 0) {
    perror("error when read psql response");
    return -1;
  }

  if (resByteslen > 0 && resBuffer[0] == 'R') {
    perror("you need auth");
    return -1;
  }

  close(socket_fd);
  return 0;
}

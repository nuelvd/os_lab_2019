#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <errno.h>
#include <getopt.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <sys/types.h>

struct Server {
  char ip[255];
  int port;
};

uint64_t MultModulo(uint64_t a, uint64_t b, uint64_t mod) {
  uint64_t result = 0;
  a = a % mod;
  while (b > 0) {
    if (b % 2 == 1)
      result = (result + a) % mod;
    a = (a * 2) % mod;
    b /= 2;
  }

  return result % mod;
}

bool ConvertStringToUI64(const char *str, uint64_t *val) {
  char *end = NULL;
  unsigned long long i = strtoull(str, &end, 10);
  if (errno == ERANGE) {
    fprintf(stderr, "Out of uint64_t range: %s\n", str);
    return false;
  }

  if (errno != 0)
    return false;

  *val = i;
  return true;
}

int main(int argc, char **argv) {
  uint64_t k = -1;
  uint64_t mod = -1;
  char servers[255] = {'\0'}; // TODO: explain why 255

  while (true) {
    int current_optind = optind ? optind : 1;

    static struct option options[] = {{"k", required_argument, 0, 0},
                                      {"mod", required_argument, 0, 0},
                                      {"servers", required_argument, 0, 0},
                                      {0, 0, 0, 0}};

    int option_index = 0;
    int c = getopt_long(argc, argv, "", options, &option_index);

    if (c == -1)
      break;

    switch (c) {
    case 0: {
      switch (option_index) {
      case 0:
        // TODO: your code here

          if (!ConvertStringToUI64(optarg, &k)) {
            fprintf(stderr, "Invalid k\n");
            return 1;
          }
          break;

        break;
      case 1:
        // TODO: your code here

        if (!ConvertStringToUI64(optarg, &mod)) {
          fprintf(stderr, "Invalid mod\n");
          return 1;
        }
        break;
      case 2:
        // TODO: your code here

        if (strlen(optarg) >= sizeof(servers)) {
          fprintf(stderr, "servers path too long\n");
          return 1;
        }

        memcpy(servers, optarg, strlen(optarg)+1);
        break;
      default:
        printf("Index %d is out of options\n", option_index);
      }
    } break;

    case '?':
      printf("Arguments error\n");
      break;
    default:
      fprintf(stderr, "getopt returned character code 0%o?\n", c);
    }
  }

  if (k == -1 || mod == -1 || !strlen(servers)) {
    fprintf(stderr, "Using: %s --k 1000 --mod 5 --servers /path/to/file\n",
            argv[0]);
    return 1;
  }

  // TODO: for one server here, rewrite with servers from file

  if (mod == 0) {
    fprintf(stderr, "mod must be > 0\n");
    return 1;
  }
  //Чтение серверов из файла
  FILE *f = fopen(servers, "r");
  if (!f) {
    fprintf(stderr, "Cannot open servers file: %s\n", servers);
    return 1;
  }
  unsigned int servers_num = 0;
  unsigned int capacity = 4;
  struct Server *to = malloc(sizeof(struct Server) * capacity);
  if (!to) {
    fprintf(stderr, "malloc failed\n");
    return 1;
  }

  char line[512];
  while (fgets(line, sizeof(line), f) != NULL) {
  char *colon = strchr(line, ':');
  if (colon == NULL)
    continue;
  *colon = '\0';
  to = realloc(to, sizeof(struct Server) * (servers_num + 1));
  snprintf(to[servers_num].ip, sizeof(to[servers_num].ip), "%s", line);
  to[servers_num].port = atoi(colon + 1);
  servers_num++;
  }
  fclose(f);
  if (servers_num == 0) {
    fprintf(stderr, "No servers in file\n");
    free(to);
    return 1;
  }

  // TODO: delete this and parallel work between servers

  uint64_t total_count = k;
  uint64_t chunk = total_count / servers_num;
  uint64_t remainder = total_count % servers_num;
  uint64_t current = 1;

  uint64_t total = 1;

  // TODO: work continiously, rewrite to make parallel

  for (int i = 0; i < servers_num; i++) {
    uint64_t my_count = chunk + (i<remainder ? 1 : 0);
    uint64_t begin = current;
    uint64_t end = current + my_count -1;
    current+=my_count;

    struct hostent *hostname = gethostbyname(to[i].ip);
    if (hostname == NULL) {
      fprintf(stderr, "gethostbyname failed with %s\n", to[i].ip);
      exit(1);
    }

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(to[i].port);
    server.sin_addr.s_addr = *((unsigned long *)hostname->h_addr);

    int sck = socket(AF_INET, SOCK_STREAM, 0);
    if (sck < 0) {
      fprintf(stderr, "Socket creation failed!\n");
      exit(1);
    }

    if (connect(sck, (struct sockaddr *)&server, sizeof(server)) < 0) {
      fprintf(stderr, "Connection failed\n");
      exit(1);
    }

    // TODO: for one server
    // parallel between servers

    char task[sizeof(uint64_t) * 3];
    memcpy(task, &begin, sizeof(uint64_t));
    memcpy(task + sizeof(uint64_t), &end, sizeof(uint64_t));
    memcpy(task + 2 * sizeof(uint64_t), &mod, sizeof(uint64_t));

    if (send(sck, task, sizeof(task), 0) < 0) {
      fprintf(stderr, "Send failed\n");
      exit(1);
    }

    char response[sizeof(uint64_t)];
    ssize_t got = 0;
    while (got < (ssize_t)sizeof(response)) {
      ssize_t n = recv(sck, response + got, sizeof(response) - got, 0);
      if (n <= 0) {
        fprintf(stderr, "Recieve failed\n");
        close(sck);
        free(to);
        exit(1);
      }
      got += n;
    }

    // TODO: from one server
    // unite results
    uint64_t answer = 0;
    memcpy(&answer, response, sizeof(uint64_t));
    fprintf(stderr, "Server %s:%d -> [%llu..%llu] = %llu\n",
            to[i].ip, to[i].port,
            (unsigned long long)begin, (unsigned long long)end,
            (unsigned long long)answer);

    total = MultModulo(total, answer, mod);

    close(sck);
  }

  printf("answer: %llu\n", (unsigned long long)total);
  free(to);

  return 0;
}

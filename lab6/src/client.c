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
#include <sys/wait.h>

#include "multmodulo.h"

struct Server {
  char ip[255];
  int port;
};

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
  char servers_file[255] = {'\0'};

  while (true) {
    int current_optind = optind ? optind : 1;
    static struct option options[] = {
      {"k", required_argument, 0, 0},
      {"mod", required_argument, 0, 0},
      {"servers", required_argument, 0, 0},
      {0, 0, 0, 0}
    };

    int option_index = 0;
    int c = getopt_long(argc, argv, "", options, &option_index);

    if (c == -1)
      break;

    switch (c) {
    case 0: {
      switch (option_index) {
      case 0:
        ConvertStringToUI64(optarg, &k);
        if (k <= 0) {
          printf("k is a positive number\n");
          return 1;
        }
        break;
      case 1:
        ConvertStringToUI64(optarg, &mod);
        if (mod <= 0) {
          printf("mod is a positive number\n");
          return 1;
        }
        break;
      case 2:
        strncpy(servers_file, optarg, sizeof(servers_file) - 1);
        servers_file[sizeof(servers_file) - 1] = '\0';
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

  if (k == -1 || mod == -1 || !strlen(servers_file)) {
    fprintf(stderr, "Using: %s --k 1000 --mod 5 --servers /path/servers/file\n",
            argv[0]);
    return 1;
  }

  // код для чтения списка серверов из файла.
  if (k == -1 || mod == -1 || strlen(servers_file) == 0) {
    fprintf(stderr, "Usage: %s --k <num> --mod <num> --servers <file>\n", argv[0]);
    return 1;
  }

  FILE *file = fopen(servers_file, "r");
  if (file == NULL) {
    perror("Failed servers open servers file");
    return 1;
  }

  char line[256];
  int servers_num = 0;
  while (fgets(line, sizeof(line), file)) {
    servers_num++;
  }

  rewind(file);

  struct Server *servers = malloc(sizeof(struct Server) * servers_num);
  for (int i = 0; i < servers_num; i++) {
    if (fgets(line, sizeof(line), file)) {
      sscanf(line, "%[^:]:%d", servers[i].ip, &servers[i].port);
    }
  }

  fclose(file);

  // struct Server *to = malloc(sizeof(struct Server) * servers_num);
  // to[0].port = 20001;
  // strcpy(to[0].ip, "127.0.0.1");
  // to[1].port = 20002;
  // strcpy(to[1].ip, "127.0.0.1");

  int fds[servers_num][2]; // Каналы для каждого дочернего процесса
  int active_child_processes = 0;

  for (int i = 0; i < servers_num; i++) {
    if (pipe(fds[i]) == -1) {
      fprintf(stderr, "Pipe Failed");
      return 1;
    }

    pid_t child_pid = fork();
    if (child_pid >= 0) {
      active_child_processes++;
      if (child_pid == 0) {
        close(fds[i][0]); // Закрыть чтение в дочернем процессе

        struct hostent *hostname = gethostbyname(servers[i].ip);
        if (hostname == NULL) {
          fprintf(stderr, "gethostbyname failed with %s\n", servers[i].ip);
          exit(1);
        }

        struct sockaddr_in server;
        server.sin_family = AF_INET;
        server.sin_port = htons(servers[i].port);
        server.sin_addr.s_addr = *((unsigned long *)hostname->h_addr_list[0]);

        int sck = socket(AF_INET, SOCK_STREAM, 0);
        if (sck < 0) {
          fprintf(stderr, "Socket creation failed!\n");
          exit(1);
        }

        if (connect(sck, (struct sockaddr *)&server, sizeof(server)) < 0) {
          fprintf(stderr, "Connection failed\n");
          exit(1);
        }

        uint64_t begin = i * k / servers_num + 1;
        uint64_t end = (i + 1) * k / servers_num + 1;
        char task[sizeof(uint64_t) * 3];
        memcpy(task, &begin, sizeof(uint64_t));
        memcpy(task + sizeof(uint64_t), &end, sizeof(uint64_t));
        memcpy(task + 2 * sizeof(uint64_t), &mod, sizeof(uint64_t));

        if (send(sck, task, sizeof(task), 0) < 0) {
          fprintf(stderr, "Send failed\n");
          exit(1);
        }
        char response[sizeof(uint64_t)];
        if (recv(sck, response, sizeof(response), 0) < 0) {
          fprintf(stderr, "Receive failed\n");
          exit(1);
        }

        uint64_t answer = 0;
        memcpy(&answer, response, sizeof(uint64_t));
        write(fds[i][1], &answer, sizeof(answer)); // Отправить ответ в родительский процесс
        close(fds[i][1]);
        close(sck);
        exit(0);
      } else {
        close(fds[i][1]); // Закрыть запись в родительском процессе
      }
    } else {
      printf("Fork failed!\n");
      return 1;
    }
  }

  uint64_t answerall = 1;
  for (int i = 0; i < servers_num; i++) {
    uint64_t answer = 0;
    read(fds[i][0], &answer, sizeof(answer)); // Чтение ответа от дочернего процесса
    answerall = (answerall * answer) % mod; // Объединение результатов
    close(fds[i][0]);
  }

  printf("answerall: %lu\n", answerall);

  while (active_child_processes > 0) {
    wait(NULL);
    active_child_processes--;
  }

  free(servers);
  return 0;
}
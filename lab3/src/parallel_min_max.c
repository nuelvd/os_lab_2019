#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <getopt.h>

#include "find_min_max.h"
#include "utils.h"

int main(int argc, char **argv) {
  int seed = -1;
  int array_size = -1;
  int pnum = -1;
  int timeout = -1;
  bool with_files = false;

  while (true) {
    int current_optind = optind ? optind : 1;

    static struct option options[] = {{"seed", required_argument, 0, 0},
                                      {"array_size", required_argument, 0, 0},
                                      {"pnum", required_argument, 0, 0},
                                      {"by_files", no_argument, 0, 'f'},
                                      {"timeout", required_argument, 0, 0},
                                      {0, 0, 0, 0}};

    int option_index = 0;
    int c = getopt_long(argc, argv, "f", options, &option_index);

    if (c == -1) break;

    switch (c) {
      case 0:
        switch (option_index) {
          case 0:
            seed = atoi(optarg);
            if (seed <= 0) { printf("invalid seed\n"); return 1; }
            break;
          case 1:
            array_size = atoi(optarg);
            if (array_size <= 0) { printf("invalid size\n"); return 1; }
            break;
          case 2:
            pnum = atoi(optarg);
            if (pnum <= 0) { printf("invalid pnum\n"); return 1; }
            break;
          case 3:
            with_files = true;
            break;
          case 4:
            timeout = atoi(optarg);
            if (timeout <= 0) { printf("invalid timeout\n"); return 1; }
            break;
          default:
            printf("Index %d is out of options\n", option_index);
        }
        break;
      case 'f':
        with_files = true;
        break;
      case '?':
        break;
      default:
        printf("getopt returned character code 0%o?\n", c);
    }
  }

  if (optind < argc) {
    printf("Has at least one no option argument\n");
    return 1;
  }

  if (seed == -1 || array_size == -1 || pnum == -1) {
    printf("Usage: %s --seed \"num\" --array_size \"num\" --pnum \"num\" "
           "[--timeout \"num\"]\n", argv[0]);
    return 1;
  }

  int *array = malloc(sizeof(int) * array_size);
  GenerateArray(array, array_size, seed);
  int active_child_processes = 0;
  pid_t *child_pids = malloc(sizeof(pid_t) * pnum);
  for (int i = 0; i < pnum; i++) child_pids[i] = -1;

  struct timeval start_time;
  gettimeofday(&start_time, NULL);

  int fd[2];
  if (!with_files) {
    if (pipe(fd) == -1) { perror("pipe"); return 1; }
  }

  for (int i = 0; i < pnum; i++) {
    pid_t child_pid = fork();
    if (child_pid >= 0) {
      active_child_processes += 1;
      if (child_pid == 0) {
        int start = i * (array_size / pnum);
        int end = (i == pnum - 1) ? array_size : start + (array_size / pnum);
        struct MinMax localmin_max = GetMinMax(array, start, end);
        sleep(2);
        if (with_files) {
          char filename[64];
          snprintf(filename, sizeof(filename), "child_%d.txt", i);
          FILE *f = fopen(filename, "w");
          if (!f) { perror("fopen"); return 1; }
          fprintf(f, "%d %d", localmin_max.min, localmin_max.max);
          fclose(f);
        } else {
          close(fd[0]);
          write(fd[1], &localmin_max.min, sizeof(int));
          write(fd[1], &localmin_max.max, sizeof(int));
          close(fd[1]);
        }
        return 0;
      } else {
        child_pids[i] = child_pid;
      }
    } else {
      printf("Fork failed!\n");
      return 1;
    }
  }

  if (!with_files) close(fd[1]);

  bool timed_out = false;
  while (active_child_processes > 0) {
    pid_t wpid;
    while ((wpid = waitpid(-1, NULL, WNOHANG)) > 0) {
      active_child_processes -= 1;
    }
    if (active_child_processes == 0) break;

    if (timeout > 0) {
      struct timeval now;
      gettimeofday(&now, NULL);
      double elapsed_ms = (now.tv_sec - start_time.tv_sec) * 1000.0 +
                          (now.tv_usec - start_time.tv_usec) / 1000.0;
      if (elapsed_ms >= timeout * 1000.0) {
        timed_out = true;
        break;
      }
    }
    usleep(1000);
  }

  if (timed_out) {
    for (int i = 0; i < pnum; i++) {
      if (child_pids[i] > 0) kill(child_pids[i], SIGKILL);
    }
    while (wait(NULL) > 0) {}
    active_child_processes = 0;
  }

  struct MinMax min_max;
  min_max.min = INT_MAX;
  min_max.max = INT_MIN;

  for (int i = 0; i < pnum; i++) {
    int min = INT_MAX;
    int max = INT_MIN;

    if (with_files) {
      char filename[64];
      snprintf(filename, sizeof(filename), "child_%d.txt", i);
      FILE *f = fopen(filename, "r");
      if (f) {
        fscanf(f, "%d %d", &min, &max);
        fclose(f);
        remove(filename);
      }
    } else {
      read(fd[0], &min, sizeof(int));
      read(fd[0], &max, sizeof(int));
    }

    if (min < min_max.min) min_max.min = min;
    if (max > min_max.max) min_max.max = max;
  }

  struct timeval finish_time;
  gettimeofday(&finish_time, NULL);

  double elapsed_time = (finish_time.tv_sec - start_time.tv_sec) * 1000.0;
  elapsed_time += (finish_time.tv_usec - start_time.tv_usec) / 1000.0;

  free(array);
  free(child_pids);

  printf("Min: %d\n", min_max.min);
  printf("Max: %d\n", min_max.max);
  printf("Elapsed time: %fms\n", elapsed_time);
  fflush(NULL);
  return 0;
}
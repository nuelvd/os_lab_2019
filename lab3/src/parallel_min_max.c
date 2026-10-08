#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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
  bool with_files = false;

  while (true) {
    int current_optind = optind ? optind : 1;

    static struct option options[] = {{"seed", required_argument, 0, 0},
                                      {"array_size", required_argument, 0, 0},
                                      {"pnum", required_argument, 0, 0},
                                      {"by_files", no_argument, 0, 'f'},
                                      {0, 0, 0, 0}};

    int option_index = 0;
    int c = getopt_long(argc, argv, "f", options, &option_index);

    if (c == -1) break;

    switch (c) {
      case 0:
        switch (option_index) {
          case 0:
            seed = atoi(optarg);
            // your code here
            // error handling
            if (seed<=0) {printf("invalid seed");return 1;}
            break;
          case 1:
            array_size = atoi(optarg);
            // your code here
            // error handling
            if (array_size<=0) {printf("invalid size");return 1;}
            break;
          case 2:
            pnum = atoi(optarg);
            // your code here
            // error handling
            if (pnum<=0) {printf("invalid pnum");return 1;}
            break;
          case 3:
            with_files = true;
            break;

          defalut:
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
    printf("Usage: %s --seed \"num\" --array_size \"num\" --pnum \"num\" \n",
           argv[0]);
    return 1;
  }

  int *array = malloc(sizeof(int) * array_size);
  GenerateArray(array, array_size, seed);
  int active_child_processes = 0;

  struct timeval start_time;
  gettimeofday(&start_time, NULL);

  int fd[2];
  if (!with_files) {
    if (pipe(fd)==-1) {perror("pipe"); return 1;}
  }

  for (int i = 0; i < pnum; i++) {
    pid_t child_pid = fork();
    if (child_pid >= 0) {
      // successful fork
      active_child_processes += 1;
      if (child_pid == 0) {
        // child process

        // parallel somehow
        int start =i *(array_size / pnum);
        int end = (i==pnum-1) ? array_size: start + (array_size / pnum);
        struct MinMax localmin_max = GetMinMax(array, start, end);
        if (with_files) {
          // use files here
          char filename[64];
          snprintf(filename, sizeof(filename), "child_%d.txt",i);
          FILE *f = fopen(filename,"r");
          fscanf(f,"%d %d", &localmin_max.min,&localmin_max.max);
          fclose(f);
        } else {
          // use pipe here
          close(fd[0]);
          write(fd[1],&localmin_max.min,sizeof(int));
          write(fd[1],&localmin_max.max,sizeof(int));
          close(fd[1]);
        }
        return 0;
      }

    } else {
      printf("Fork failed!\n");
      return 1;
    }
  }

  if (!with_files) close(fd[1]);

  while (active_child_processes > 0) {
    // your code here
    wait(NULL);
    active_child_processes -= 1;
  }

  struct MinMax min_max;
  min_max.min = INT_MAX;
  min_max.max = INT_MIN;

  for (int i = 0; i < pnum; i++) {
    int min = INT_MAX;
    int max = INT_MIN;

    if (with_files) {
      // read from files
      char filename[64];
          snprintf(filename, sizeof(filename), "child_%d.txt",i);
          FILE *f = fopen(filename,"r");
          if (f) {
            fscanf(f,"%d %d", &min,&max);
            fclose(f);
            remove(filename);
          }
    } else {
      // read from pipes
      read(fd[0],&min, sizeof(int));
      read(fd[0],&max, sizeof(int));
    }

    if (min < min_max.min) min_max.min = min;
    if (max > min_max.max) min_max.max = max;
  }

  struct timeval finish_time;
  gettimeofday(&finish_time, NULL);

  double elapsed_time = (finish_time.tv_sec - start_time.tv_sec) * 1000.0;
  elapsed_time += (finish_time.tv_usec - start_time.tv_usec) / 1000.0;

  free(array);

  printf("Min: %d\n", min_max.min);
  printf("Max: %d\n", min_max.max);
  printf("Elapsed time: %fms\n", elapsed_time);
  fflush(NULL);
  return 0;
}

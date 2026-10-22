#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <getopt.h>
#include <pthread.h>

#include "sum_lib.h"

static void GenerateArray(int *array, unsigned int array_size, unsigned int seed) {
  srand(seed);
  for (unsigned int i = 0; i < array_size; i++) {
    array[i] = rand();
  }
}

struct ThreadResult {
  struct SumArgs args;
  long long sum;
};

static void *ThreadSum(void *arg) {
  struct ThreadResult *r = (struct ThreadResult *)arg;
  r->sum = Sum(&r->args);
  return NULL;
}

static double now_ms(void) {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return ts.tv_sec * 1000.0 + ts.tv_nsec / 1e6;
}

int main(int argc, char **argv) {
  uint32_t threads_num = 0;
  uint32_t array_size = 0;
  uint32_t seed = 0;

  static struct option options[] = {
      {"threads_num", required_argument, 0, 0},
      {"array_size",  required_argument, 0, 0},
      {"seed",        required_argument, 0, 0},
      {0, 0, 0, 0}};

  int option_index = 0, c;
  while ((c = getopt_long(argc, argv, "", options, &option_index)) != -1) {
    if (c != 0) continue;
    switch (option_index) {
      case 0:
        threads_num = (uint32_t)atoi(optarg);
        if (threads_num == 0) { printf("invalid threads_num\n"); return 1; }
        break;
      case 1:
        array_size = (uint32_t)atoi(optarg);
        if (array_size == 0) { printf("invalid array_size\n"); return 1; }
        break;
      case 2:
        seed = (uint32_t)atoi(optarg);
        break;
      default:
        printf("unknown option\n"); return 1;
    }
  }

  if (threads_num == 0 || array_size == 0) {
    printf("Usage: %s --threads_num \"num\" --array_size \"num\" --seed \"num\"\n",
           argv[0]);
    return 1;
  }

  int *array = malloc(sizeof(int) * array_size);
  if (!array) { perror("malloc"); return 1; }
  GenerateArray(array, array_size, seed);

  pthread_t *threads = malloc(sizeof(pthread_t) * threads_num);
  struct ThreadResult *results = malloc(sizeof(struct ThreadResult) * threads_num);
  if (!threads || !results) { perror("malloc"); return 1; }

  uint32_t base = array_size / threads_num;
  uint32_t rem  = array_size % threads_num;

  uint32_t offset = 0;
  for (uint32_t i = 0; i < threads_num; i++) {
    uint32_t chunk = base + (i < rem ? 1 : 0);
    results[i].args.array = array;
    results[i].args.begin = (int)offset;
    results[i].args.end   = (int)(offset + chunk);
    results[i].sum = 0;
    offset += chunk;
  }

  double t0 = now_ms();

  for (uint32_t i = 0; i < threads_num; i++) {
    if (pthread_create(&threads[i], NULL, ThreadSum, &results[i]) != 0) {
      printf("Error: pthread_create failed!\n");
      return 1;
    }
  }

  long long total_sum = 0;
  for (uint32_t i = 0; i < threads_num; i++) {
    pthread_join(threads[i], NULL);
    total_sum += results[i].sum;
  }

  double t1 = now_ms();

  printf("Total: %lld\n", total_sum);
  printf("Elapsed time: %.3f ms\n", t1 - t0);
  printf("Threads: %u, array_size: %u, seed: %u\n",
         threads_num, array_size, seed);

  free(threads);
  free(results);
  free(array);
  return 0;
}
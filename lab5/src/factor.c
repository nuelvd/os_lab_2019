#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <getopt.h>

long long k, pnum, mod;
long long result = 1;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct {
    long long start;
    long long end;
} ThreadArg;

void* compute_factorial(void* arg) {
    ThreadArg* range = (ThreadArg*)arg;
    long long local_result = 1;
    for (long long i = range->start; i <= range->end; i++) {
        local_result = (local_result * i) % mod;
    }

    pthread_mutex_lock(&mutex);
    result = (result * local_result) % mod;
    pthread_mutex_unlock(&mutex);

    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    k = 0;
    pnum = 1;
    mod = 1;

    static struct option long_options[] = {
        {"pnum", required_argument, 0, 'p'},
        {"mod",  required_argument, 0, 'm'},
        {0, 0, 0, 0}
    };

    int opt;
    int option_index = 0;
    while ((opt = getopt_long(argc, argv, "k:", long_options, &option_index)) != -1) {
        switch (opt) {
            case 'k':
                k = atoll(optarg);
                break;
            case 'p':
                pnum = atoll(optarg);
                break;
            case 'm':
                mod = atoll(optarg);
                break;
        }
    }


    pthread_t* threads = malloc(pnum * sizeof(pthread_t));
    ThreadArg* args = malloc(pnum * sizeof(ThreadArg));

    long long chunk_size = (k + pnum - 1) / pnum;
    for (long long i = 0; i < pnum; i++) {
        args[i].start = i * chunk_size + 1;
        args[i].end = (i + 1) * chunk_size;
        if (args[i].end > k) {
            args[i].end = k;
        }
        if (args[i].start > args[i].end) {
            args[i].start = 1;
            args[i].end = 0;
        }

        if (pthread_create(&threads[i], NULL, compute_factorial, &args[i]) != 0) {
            perror("Thread creation failed");
            exit(EXIT_FAILURE);
        }
    }

    for (long long i = 0; i < pnum; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("%lld! mod %lld = %lld\n", k, mod, result);

    free(threads);
    free(args);

    return 0;
}
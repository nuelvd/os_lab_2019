#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <getopt.h>

// Глобальные переменные
long long k, pnum, mod;
long long result = 1; // Общий результат
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

// Структура для передачи аргументов в поток
typedef struct {
    long long start;
    long long end;
} ThreadArg;

// Функция потока
void* compute_factorial(void* arg) {
    ThreadArg* range = (ThreadArg*)arg;
    long long local_result = 1;

    // Вычисляем произведение своего диапазона
    for (long long i = range->start; i <= range->end; i++) {
        local_result = (local_result * i) % mod;
    }

    // Синхронизация: обновляем общий результат под мьютексом
    pthread_mutex_lock(&mutex);
    result = (result * local_result) % mod;
    pthread_mutex_unlock(&mutex);

    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    // Значения по умолчанию
    k = 0;
    pnum = 1;
    mod = 1;

    // Разбор аргументов командной строки
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

    // Распределение диапазонов между потоками
    long long chunk_size = (k + pnum - 1) / pnum; // Округление вверх
    for (long long i = 0; i < pnum; i++) {
        args[i].start = i * chunk_size + 1;
        args[i].end = (i + 1) * chunk_size;
        if (args[i].end > k) {
            args[i].end = k;
        }
        // Если диапазон пустой (например, при k=0 или малом k), поток завершится сразу
        if (args[i].start > args[i].end) {
            args[i].start = 1;
            args[i].end = 0; // Цикл не выполнится
        }

        // Создание потока
        if (pthread_create(&threads[i], NULL, compute_factorial, &args[i]) != 0) {
            perror("Thread creation failed");
            exit(EXIT_FAILURE);
        }
    }

    // Ожидание завершения всех потоков
    for (long long i = 0; i < pnum; i++) {
        pthread_join(threads[i], NULL);
    }

    // Вывод результата
    printf("%lld! mod %lld = %lld\n", k, mod, result);

    free(threads);
    free(args);

    return 0;
}
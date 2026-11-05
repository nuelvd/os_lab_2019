#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

// Два мьютекса — два "ресурса"
pthread_mutex_t mutex_a = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_b = PTHREAD_MUTEX_INITIALIZER;

// Поток 1: сначала захватывает A, потом B
void* thread1_func(void* arg) {
    printf("[Thread 1] Пытается захватить mutex A...\n");
    pthread_mutex_lock(&mutex_a);
    printf("[Thread 1] Захватил mutex A\n");

    sleep(1); // Даём потоку 2 время захватить B

    printf("[Thread 1] Пытается захватить mutex B...\n");
    pthread_mutex_lock(&mutex_b);   // <-- Здесь поток 1 зависнет
    printf("[Thread 1] Захватил mutex B\n");

    // Критическая секция
    pthread_mutex_unlock(&mutex_b);
    pthread_mutex_unlock(&mutex_a);
    printf("[Thread 1] Завершился\n");
    return NULL;
}

// Поток 2: сначала захватывает B, потом A (обратный порядок!)
void* thread2_func(void* arg) {
    printf("[Thread 2] Пытается захватить mutex B...\n");
    pthread_mutex_lock(&mutex_b);
    printf("[Thread 2] Захватил mutex B\n");

    sleep(1); // Даём потоку 1 время захватить A

    printf("[Thread 2] Пытается захватить mutex A...\n");
    pthread_mutex_lock(&mutex_a);   // <-- Здесь поток 2 зависнет
    printf("[Thread 2] Захватил mutex A\n");

    pthread_mutex_unlock(&mutex_a);
    pthread_mutex_unlock(&mutex_b);
    printf("[Thread 2] Завершился\n");
    return NULL;
}

int main(void) {
    pthread_t t1, t2;

    printf("=== Демонстрация deadlock ===\n");

    pthread_create(&t1, NULL, thread1_func, NULL);
    pthread_create(&t2, NULL, thread2_func, NULL);

    // main будет вечно ждать завершения потоков, которые никогда не завершатся
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    printf("Эта строка никогда не будет выведена\n");

    pthread_mutex_destroy(&mutex_a);
    pthread_mutex_destroy(&mutex_b);
    return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

pthread_mutex_t mutex_a = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_b = PTHREAD_MUTEX_INITIALIZER;

void* thread1_func(void* arg) {
    printf("(Thread 1) захват mutex A\n");
    pthread_mutex_lock(&mutex_a);
    printf("(Thread 1) захватил mutex A\n");

    sleep(1);

    printf("(Thread 1) захват mutex B\n");
    pthread_mutex_lock(&mutex_b);
    printf("(Thread 1) захватил mutex B\n");

    pthread_mutex_unlock(&mutex_b);
    pthread_mutex_unlock(&mutex_a);
    printf("(Thread 1) конец\n");
    return NULL;
}

void* thread2_func(void* arg) {
    printf("(Thread 2) захват mutex B...\n");
    pthread_mutex_lock(&mutex_b);
    printf("(Thread 2) Захватил mutex B\n");

    sleep(1);

    printf("(Thread 2) захват mutex A\n");
    pthread_mutex_lock(&mutex_a);
    printf("(Thread 2) захватил mutex A\n");

    pthread_mutex_unlock(&mutex_a);
    pthread_mutex_unlock(&mutex_b);
    printf("(Thread 2) конец\n");
    return NULL;
}

int main(void) {
    pthread_t t1, t2;

    pthread_create(&t1, NULL, thread1_func, NULL);
    pthread_create(&t2, NULL, thread2_func, NULL);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    printf("строка\n");

    pthread_mutex_destroy(&mutex_a);
    pthread_mutex_destroy(&mutex_b);
    return 0;
}
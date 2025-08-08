#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include "monitor.h"

#define SIZE 5

typedef struct
{
    monitor_t *monitor;
    int thread_id;
} thread_data_t;

void *worker_thread(void *arg)
{
    thread_data_t *data = (thread_data_t *)arg;

    printf("Thread %d: Waiting on monitor...\n", data->thread_id);
    monitor_wait(data->monitor);
    printf("Thread %d: Woke up!\n", data->thread_id);

    return NULL;
}

void test_multiple_threads()
{

    monitor_t monitor;
    monitor_init(&monitor);

    pthread_t threads[SIZE];
    thread_data_t thread_data[SIZE];

    for (int i = 0; i < SIZE; i++)
    {

        thread_data[i].monitor = &monitor;
        thread_data[i].thread_id = i;
        pthread_create(&threads[i], NULL, worker_thread, &thread_data[i]);
    }
    sleep(1);

    printf("Main: Signaling monitor\n");
    monitor_signal(&monitor);

    for (int i = 0; i < 5; i++)
    {
        pthread_join(threads[i], NULL);
    }

    monitor_destroy(&monitor);
}
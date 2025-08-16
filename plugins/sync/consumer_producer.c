#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "consumer_producer.h"

const char *consumer_producer_init(consumer_producer_t *queue, int capacity)
{
    if (queue == NULL)
    {
        return "Queue pointer is NULL";
    }

    if (capacity <= 0)
    {
        return "Queue capacity can only be a positive number";
    }

    queue->items = malloc(capacity * sizeof(char *));
    if (queue->items == NULL)
    {
        return "Failed to allocate memory for items array";
    }

    for (int i = 0; i < capacity; i++)
    {
        queue->items[i] = NULL;
    }

    queue->capacity = capacity;
    queue->count = 0;
    queue->head = 0;
    queue->tail = 0;

    if (monitor_init(&queue->not_empty_monitor) != 0)
    {
        return "Failed to initialize not_empty_monitor";
    }

    if (monitor_init(&queue->not_full_monitor) != 0)
    {
        monitor_destroy(&queue->not_empty_monitor);
        return "Failed to initialize not_full_monitor";
    }

    if (monitor_init(&queue->finished_monitor) != 0)
    {
        monitor_destroy(&queue->not_empty_monitor);
        monitor_destroy(&queue->not_full_monitor);
        return "Failed to initialize finished_monitor";
    }

    if (pthread_mutex_init(&queue->queue_lock, NULL) != 0)
    {
        monitor_destroy(&queue->not_empty_monitor);
        monitor_destroy(&queue->not_full_monitor);
        monitor_destroy(&queue->finished_monitor);
        return "Failed to initialize queue mutex";
    }

    return NULL;
}

void consumer_producer_destroy(consumer_producer_t *queue)
{
    monitor_destroy(&queue->finished_monitor);
    monitor_destroy(&queue->not_full_monitor);
    monitor_destroy(&queue->not_empty_monitor);
    pthread_mutex_destroy(&queue->queue_lock);
    free(queue->items);
}

const char *consumer_producer_put(consumer_producer_t *queue, const char *item)
{
    if (queue == NULL)
    {
        return "Null Queue pointer";
    }

    if (item == NULL)
    {
        return "NULL Item pointer";
    }

    if (queue->finished_monitor.signaled == 1)
    {
        return "Can't add items after finish";
    }

    pthread_mutex_lock(&queue->queue_lock);

    while (queue->count >= queue->capacity)
    {
        monitor_reset(&queue->not_full_monitor);
        pthread_mutex_unlock(&queue->queue_lock);
        monitor_wait(&queue->not_full_monitor);
        pthread_mutex_lock(&queue->queue_lock);
    }

    queue->items[queue->tail] = strdup(item);
    if (queue->items[queue->tail] == NULL)
    {
        pthread_mutex_unlock(&queue->queue_lock);
        return "Error: Memory allocation for string failed";
    }

    queue->tail = (queue->tail + 1) % queue->capacity;
    queue->count++;

    monitor_signal(&queue->not_empty_monitor);

    pthread_mutex_unlock(&queue->queue_lock);

    return NULL;
}

char *consumer_producer_get(consumer_producer_t *queue)
{
    if (queue == NULL)
    {
        return "Error: NULL Queue pointer";
    }

    pthread_mutex_lock(&queue->queue_lock);

    while (queue->count <= 0 && queue->finished_monitor.signaled == 0)
    {
        monitor_reset(&queue->not_empty_monitor);
        pthread_mutex_unlock(&queue->queue_lock);
        monitor_wait(&queue->not_empty_monitor);
        pthread_mutex_lock(&queue->queue_lock);
    }

    if (queue->count <= 0 && queue->finished_monitor.signaled == 1)
    {
        pthread_mutex_unlock(&queue->queue_lock);
        return NULL;
    }

    char *returnVal = queue->items[queue->head];
    queue->items[queue->head] = NULL;

    queue->head = (queue->head + 1) % queue->capacity;
    queue->count--;

    monitor_signal(&queue->not_full_monitor);

    pthread_mutex_unlock(&queue->queue_lock);

    return returnVal;
}

void consumer_producer_signal_finished(consumer_producer_t *queue)
{
    if (queue == NULL)
    {
        return;
    }

    monitor_signal(&queue->finished_monitor);
    monitor_signal(&queue->not_empty_monitor);
}

int consumer_producer_wait_finished(consumer_producer_t *queue)
{
    if (queue == NULL)
    {
        return -1;
    }
    monitor_wait(&queue->finished_monitor);
    return 0;
}

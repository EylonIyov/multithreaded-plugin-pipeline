#include "consumer_producer.h"

pthread_mutex_t queue_lock;

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

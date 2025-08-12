#include "plugin_common.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

static plugin_context_t plugin_context;

void *plugin_consumer_thread(void *arg)
{
    plugin_context_t *context = (plugin_context_t *)arg;

    if (!context || !context->queue || !context->process_function)
    {
        return NULL;
    }

    context->initialized = 1;

    while (!context->finished)
    {
        char *input = consumer_producer_get(context->queue); // If Queue is empty, will wait for queue to fill up here

        if (!input)
        {
            break; // Consumer_producer_get will return NULL only when finished signal was recived
        }

        if (strcmp(input, "<END>" == 0))
        {
            if (context->next_place_work != NULL)
            {
                context->next_place_work(input);
            }
            free(input);
            context->finished = 1;
            break;
        }

        const char *output = context->process_function(input);

        if (output == NULL && context->next_place_work != NULL)
        {
            context->next_place_work(input);
        }
        else if (output != NULL)
        {
            log_error(context, output);
        }
        free(input);
    }
}

void log_error(plugin_context_t *context, const char *message)
{
    printf("[ERROR] %s - %s\n", context->name, message);
}

void log_info(plugin_context_t *context, const char *message)
{
    printf("[INFO] %s - %s\n", context->name, message);
}

const char *common_plugin_init(const char *(*process_function)(const char *), const char *name, int queue_size)
{

    // Input validation
    if (!process_function)
    {
        return "Process_function can't be NULL";
    }
    if (!name)
    {
        return "Plugin name cannot be NULL";
    }

    if (queue_size <= 0)
    {
        return "Queue size must be positive";
    }

    // Initialize the fields of the plugin
    plugin_context.name = name;
    plugin_context.process_function = process_function;
    plugin_context.next_place_work = NULL;
    plugin_context.initialized = 0;
    plugin_context.finished = 0;

    // Initialize a pointer for the plugins queue:
    plugin_context.queue = malloc(sizeof(consumer_producer_t));
    if (!plugin_context.queue)
    {
        return "Memory allocation for queue failed";
    }

    // Initiate the consumer_producer sync mechanism
    const char *error = consumer_producer_init(plugin_context.queue, queue_size);
    if (error)
    {
        free(plugin_context.queue);
        plugin_context.queue = NULL;
        return error;
    }

    // Create the consumer thread
    int thread_output = pthread_create(&plugin_context.consumer_thread, NULL, plugin_consumer_thread, &plugin_context);
    if (thread_output != 0)
    { // Thread wasn't created properly, output of the function is 0
        consumer_producer_destroy(plugin_context.queue);
        free(plugin_context.queue);
        plugin_context.queue = NULL;
        return "Creating the consumer thread failed";
    }

    // Create a small delay for the plugin to initialize
    while (!plugin_context.initialized)
    {
        usleep(500);
    }

    // If we reached here - success
    return NULL;
}
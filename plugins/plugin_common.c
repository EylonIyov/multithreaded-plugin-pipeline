#include "plugin_common.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

static plugin_context_t *plugin_context;

void *plugin_consumer_thread(void *arg)
{

    plugin_context_t *context = (plugin_context_t *)arg;
    context->initialized = 1;
    printf("[DEBUG] %s: Consumer thread started\n", plugin_context->name);

    if (!context || !context->queue || !context->process_function)
    {
        return NULL;
    }

        while (!context->finished)
    {
        printf("[DEBUG] %s: Waiting for input\n", context->name);
        char *input = consumer_producer_get(context->queue); // If Queue is empty, will wait for queue to fill up here

        if (!input)
        {
            context->finished = 1;
            break; // Consumer_producer_get will return NULL only when finished signal was recived
        }
        printf("[DEBUG] %s: Got input: %s\n", context->name, input ? input : "NULL");

        if (strcmp(input, "<END>") == 0)
        {
            consumer_producer_signal_finished(context->queue);

            if (context->next_place_work != NULL)
            {
                context->next_place_work(input);
            }
            context->finished = 1;
            free(input);

            break;
        }

        const char *output = context->process_function(input);

        free(input);

        if (output == NULL)
        {
            log_error(context, "Transformation of input failed");
            continue;
        }

        if (context->next_place_work != NULL)
        {
            const char *error = context->next_place_work(output);
            if (error != NULL)
            {
                log_error(context, error);
            }
        }
        free((void *)output);
    }

    return NULL;
}

void log_error(plugin_context_t *context, const char *message)
{
    fprintf(stderr, "[%s] %s\n", context->name, message);
}

void log_info(plugin_context_t *context, const char *message)
{
    printf("[%s] %s\n", context->name, message);
}

const char *plugin_get_name(void)
{
    return plugin_context->name;
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
    plugin_context->name = name;
    plugin_context->process_function = process_function;
    plugin_context->next_place_work = NULL;
    plugin_context->initialized = 0;
    plugin_context->finished = 0;

    // Initialize a pointer for the plugins queue:
    plugin_context->queue = malloc(sizeof(consumer_producer_t));
    if (!plugin_context->queue)
    {
        return "Memory allocation for queue failed";
    }

    // Initiate the consumer_producer sync mechanism
    const char *error = consumer_producer_init(plugin_context->queue, queue_size);
    if (error)
    {
        free(plugin_context->queue);
        plugin_context->queue = NULL;
        return error;
    }

    // Create the consumer thread
    int thread_output = pthread_create(&plugin_context->consumer_thread, NULL, plugin_consumer_thread, &plugin_context);
    if (thread_output != 0)
    { // Thread wasn't created properly, output of the function is 0
        consumer_producer_destroy(plugin_context->queue);
        free(plugin_context->queue);
        plugin_context->queue = NULL;
        return "Creating the consumer thread failed";
    }

    // If we reached here - success
    return NULL;
}

const char *plugin_fini(void)
{
    if (!plugin_context->queue)
    {
        return "Plugin not initialized yet";
    }
    // Destroy and free all resources
    pthread_join(plugin_context->consumer_thread, NULL);
    consumer_producer_destroy(plugin_context->queue);
    free(plugin_context->queue);
    plugin_context->queue = NULL;
    printf("[DEBUG] Plugin %s shutdown", plugin_context->name);
    return NULL;
}

const char *plugin_place_work(const char *str)
{
    if (!plugin_context->queue)
    {
        return "Plugin not initialized yet";
    }
    return consumer_producer_put(plugin_context->queue, str);
}

void plugin_attach(const char *(*next_place_work)(const char *))
{
    plugin_context->next_place_work = next_place_work;
}

__attribute__((visibility("default")))
const char *
plugin_wait_finished(void)
{
    if (!plugin_context->queue)
    {
        return "Plugin not initialized yet";
    }
    return consumer_producer_wait_finished(plugin_context->queue) == 0 ? NULL : "Waiting failed"; // Return an error message if waiting for finish failed
}

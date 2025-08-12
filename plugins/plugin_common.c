#include "plugin_common.h"
#include <stdio.h>
#include <pthread.h>
#include <string.h>

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

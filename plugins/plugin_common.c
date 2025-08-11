#include "plugin_common.h"
#include <stdio.h>

void *plugin_consumer_thread(void *arg)
{
}

void log_error(plugin_context_t *context, const char *message)
{
    printf("[ERROR] %s - %s", context->name, message);
}

void log_info(plugin_context_t *context, const char *message)
{
    printf("[INFO] %s - %s", context->name, message);
}
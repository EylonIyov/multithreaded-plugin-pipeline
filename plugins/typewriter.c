#include "plugin_common.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#define SECOND 500000u

const char *plugin_transform(const char *input)
{
    char *output = strdup(input);
    if (!output)
        return NULL;

    printf("[typewriter] ");
    fflush(stdout);
    for (int i = 0; output[i]; i++)
    {
        usleep(SECOND);
        putchar(output[i]);
        fflush(stdout);
    }
    putchar('\n');
    fflush(stdout);
    return output;
}

__attribute__((visibility("default")))
const char *
plugin_init(int queue_size)
{
    return common_plugin_init(plugin_transform, "typewriter", queue_size);
}
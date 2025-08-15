#include "plugin_common.h"
#include <stdio.h>

const char *plugin_transform(const char *input)
{
    char *output = strdup(input);
    if (!output)
    {
        return NULL;
    }
    fprint("[logger] %s", output);
    return output;
}

__attribute__((visibility("default")))
const char *
plugin_init(int queue_size)
{
    return common_plugin_init((void *)plugin_transform, "logger", queue_size);
}
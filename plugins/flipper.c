#include "plugin_common.h"
#include "plugin_sdk.h"
#include <stdio.h>
#include <string.h>

const char *plugin_transform(const char *input)
{
    char *output = strdup(input);
    if (!output)
        return NULL;

    int length = strlen(output);
    char temp;
    for (int i = 0; i < length / 2; i++)
    {
        temp = output[i];
        output[i] = output[length - 1 - i];
        output[length - 1 - i] = temp;
    }

    return output;
}

__attribute__((visibility("default")))
const char *
plugin_init(int queue_size)
{
    return common_plugin_init(plugin_transform, "<flipper>", queue_size);
}
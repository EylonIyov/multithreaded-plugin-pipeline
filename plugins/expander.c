#include "plugin_common.h"
#include "plugin_sdk.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *plugin_transform(const char *input)
{
    int length = strlen(input);
    if (length == 0)
    {
        char *output = malloc(1);
        if (output)
            output[0] = '\0';
        return output;
    }

    int output_size = length + (length - 1) + 1;
    char *output = malloc(output_size);

    if (!output)
        return NULL;

    int j = 0;
    for (int i = 0; i < length; i++)
    {
        output[j++] = input[i];
        if (i != length - 1)
        {
            output[j++] = ' ';
        }
    }
    output[j] = '\0';
    return output;
}

__attribute__((visibility("default")))
const char *
plugin_init(int queue_size)
{
    return common_plugin_init(plugin_transform, "expander", queue_size);
}

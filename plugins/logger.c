#include "plugin_common.h"
#include <stdio.h>
#include <string.h>

const char *plugin_transform(const char *input)
{
    char *output = strdup(input);
    if (!output)
        return NULL;
    printf("[logger] %s\n", output);

    return output;
}

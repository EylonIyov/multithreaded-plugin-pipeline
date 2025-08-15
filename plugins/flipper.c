#include "plugin_common.h"
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

#include "plugin_common.h"
#include <string.h>

const char *plugin_transform(const char *input)
{
    char *output = strdup(input);
    if (!output)
    {
        return NULL;
    }

    for (int i = 0; output[i]; i++)
    {
        if (output[i] <= 'z' && output[i] >= 'a')
        {
            output[i] = (output[i] - 'a') + 'A';
        }
    }
    return output;
}

#include "plugin_common.h"
#include <stdio.h>

const char *plugin_transform(const char *input)
{
    int length = strlen(input);
    char *output = malloc((length * 2 + 1) * sizeof(char));

    if (!output)
        return NULL;

    for (int i = 0, j = 0; i < length; i++)
    {
        output[j++] = input[i];
        if (i != length - 1)
        {
            output[j++] = ' ';
        }
    }
    output[length * 2] = '\0';
    return output;
}

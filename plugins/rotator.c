#include "plugin_common.h"
#include <string.h>

const char *plugin_transform(const char *input)
{
    char *output = strdup(input);
    if (!output)
        return NULL;

    int stringLength = strlen(output);
    if (stringLength == 1)
    {
        return output;
    }
    char lastChar = output[stringLength - 1];

    for (int i = stringLength - 1; i > 0; i--)
    {
        output[i] = output[i - 1];
    }
    output[0] = lastChar;

    return output;
}

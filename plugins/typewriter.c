#include "plugin_common.h"
#include <stdio.h>  // for printf
#include <string.h> // for strdup
#include <unistd.h> // for usleep
#include <stdlib.h> // for malloc/free

#define SECOND 100000

const char *plugin_transform(const char *input)
{
    char *output = strdup(input);
    if (!output)
        return NULL;

    printf("[typewriter] ");
    for (int i = 0; output[i]; i++)
    {
        usleep(SECOND);
        printf("%c", output[i]);
    }
    printf("\n");
    return output;
}

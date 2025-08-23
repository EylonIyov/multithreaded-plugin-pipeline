#include "plugin_sdk.h"
#include <stddef.h>
#include "plugin_common.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define SECOND 1

const char *plugin_transform(const char *input)
{
    sleep(SECOND * 5);
    if (strcmp(input, "<END>") == 0)
    {
        return input;
    }
    int len = strlen(input);
    char *aexpand_str = malloc(len + 1);
    if (!aexpand_str)
    {
        log_error(NULL, "Error: Memory allocation for the expanded string failed");
        return input;
    }

    strcpy(aexpand_str, input);
    aexpand_str[len + 1] = '\0';

    return aexpand_str;
}

const char *plugin_init(int queue_size)
{
    return common_plugin_init(plugin_transform, "<sleeplug>", queue_size);
}
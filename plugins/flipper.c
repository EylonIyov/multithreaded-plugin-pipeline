#include "plugin_common.h"
#include <stdio.h>

const char *plugin_transform(const char *input)
{
}

__attribute__((visibility("default")))
const char *
plugin_init(int queue_size)
{
    return common_plugin_init((void *)plugin_transform, "flipper", queue_size);
}
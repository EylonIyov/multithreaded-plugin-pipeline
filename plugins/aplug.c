#include "plugin_sdk.h"
#include <stddef.h>
#include "plugin_common.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


const char* plugin_transform(const char* input) {
    if (strcmp(input, "<END>") == 0) {
        return input;
    }
    int len = strlen(input);
    char* aexpand_str = malloc(len + 2); 
    if (!aexpand_str) {
        log_error(NULL, "Error: Memory allocation for the expanded string failed");
        return input;
    }

    strcpy(aexpand_str, input);       
    aexpand_str[len] = 'a';           
    aexpand_str[len + 1] = '\0';      

    return aexpand_str;
}


const char* plugin_init(int queue_size) {
    return common_plugin_init(plugin_transform, "<aplug>", queue_size);
}
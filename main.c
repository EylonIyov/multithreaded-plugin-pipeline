#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>
#include <string.h>
#include "plugins/plugin_common.h"

#define MAX_WORD_LENGTH 1024

typedef struct
{
    plugin_init_func_t init;
    plugin_fini_func_t fini;
    plugin_place_work_func_t place_work;
    plugin_attach_func_t attach;
    plugin_wait_finished_func_t wait_finished;
    char *name;
    void *handle;
} plugin_handle_t;

int main(int argc, char *argv[])
{
    // Verify first argument is a valid positive number
    if (argc < 2)
    {
        fprintf(stderr, "Error");
        printf("Usage: ./analyzer <queue_size> <plugin1> <plugin2> ... <pluginN>");
        return -1;
    }

    int queueSize = verifyInteger(argv[1]);
    if (queueSize <= 0)
    {
        fprintf(stderr, "Error");
        printf("<queue_size> must be a valid integer");
        return -1;
    }
}

// returns 0 if a number, otherwise -1
int verifyInteger(const char *str)
{
    if (*str == '\0')
    {
        return -1;
    }

    char *endptr;
    long val = strtol(str, &endptr, 10);
    if (*endptr != '\0')
    {
        return -1;
    }

    return val;
}

int pipeline_init(int pluginCount, char *pluginNamesRaw[], int queueSize)
{
    if (pluginCount <= 0)
    {
        fprintf(stderr, "error");
        printf("Needs at least one plugin to start pipeline");
        return -1;
    }

    void **handles = malloc(pluginCount * sizeof(void *));
    if (!handles)
    {
        fprintf(stderr, "[ERROR] Allocating space for handles failed");
        return -1;
    }

    char **pluginNames = transformPluginName(pluginNamesRaw, pluginCount);

    for (int i = 0; i < pluginCount; i++)
    {
        handles[i] = dlopen(pluginNames[i], RTLD_LAZY);
        if (!handles[i])
        {
            fprintf(stderr, "[ERROR] Failed to load plugin %s: %s\n", pluginNames[i], dlerror());
            for (int j = 0; j < i; j++)
            {
                dlclose(handles[j]);
            }

            // Free transformed names
            for (int j = 0; j < pluginCount; j++)
            {
                free(pluginNames[j]);
            }
            free(pluginNames);
            free(handles);
            return -1;
        }
        printf("[INFO] Loaded plugin: %s\n", pluginNames[i]);
    }

    // Implementation of Pipeline

    for (int i = 0; i < pluginCount; i++)
    {
        free(pluginNames[i]);
    }
    free(pluginNames);
}

char **transformPluginName(char **pluginNames, int count)
{

    char **pluginNamesTransformed = malloc(count * sizeof(char *));
    if (!pluginNamesTransformed)
    {
        return NULL;
    }
    for (int i = 0; i < count; i++)
    {
        pluginNamesTransformed[i] = malloc(MAX_WORD_LENGTH);
        if (!pluginNamesTransformed[i])
        {
            for (int j = 0; j < i; j++)
            {
                free(pluginNamesTransformed[j]);
            }
            free(pluginNamesTransformed);
            return NULL;
        }
        strcpy(pluginNamesTransformed[i], "/output/");
        strcat(pluginNamesTransformed[i], pluginNames[i]);
        strcat(pluginNamesTransformed[i], ".so");
    }

    return pluginNamesTransformed;
}
#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>
#include <string.h>
#include "plugins/plugin_common.h"

#define MAX_WORD_LENGTH 1024

// defenitions for the struct saving each plugins info
typedef const char *(*plugin_init_func_t)(int queue_size);
typedef void (*plugin_fini_func_t)(void);
typedef const char *(*plugin_place_work_func_t)(const char *work);
typedef void (*plugin_attach_func_t)(const char *(*next_place_work)(const char *));
typedef int (*plugin_wait_finished_func_t)(void);

// The struct as advised in the guideline
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

static plugin_handle_t *plugin_handles = NULL;
static int g_pluginCount = 0;

int main(int argc, char *argv[])
{
    // Verify first argument is a valid positive number
    if (argc < 3)
    {
        fprintf(stderr, "Error");
        printf("Usage: ./analyzer <queue_size> <plugin1> <plugin2> ... <pluginN>");
        return -1;
    }

    int queueSize = verifyInteger(argv[1]);
    if (queueSize <= 0)
    {
        fprintf(stderr, "Error");
        printf("Usage: ./analyzer <queue_size> <plugin1> <plugin2> ... <pluginN>");
        return -1;
    }
    g_pluginCount = argc - 2;

    if (pipeline_init(&argv[2], queueSize) != 0)
    {
        return -1;
    }

    for (int i = 0; i < g_pluginCount; i++)
    {
        const char *error = plugin_handles[i].init(queueSize);
        if (error)
        {
            fprintf(stderr, "[ERROR] Plugin %s init failed: %s\n", plugin_handles[i].name, error);
            pipeline_destroy();
            return -1;
        }
    }
    char readBuffer[MAX_WORD_LENGTH];
    while (fgets(readBuffer, sizeof(readBuffer), stdin))
    {
        long sizeOfBuffer = strlen(readBuffer);

        if (sizeOfBuffer > 0 && readBuffer[sizeOfBuffer - 1] == '\n')
        {
            readBuffer[sizeOfBuffer - 1] = '\0';
        }

        const char *error = plugin_handles[0].place_work(readBuffer);
        if (error)
        {
            fprintf(stderr, "[ERROR] Failed to place work in pipeline: %s\n", error);
            break;
        }

        if (strcmp(readBuffer, "<END>"))
        {
            break;
        }

        for (int i = 0; i < g_pluginCount; i++)
        {
            int returnCode = plugin_handles[i].wait_finished();
            if (returnCode != 0)
            {
                fprintf(stderr, "[ERROR] Plugin %s failed to finish properly\n", plugin_handles[i].name);
            }
        }
    }

    pipeline_destroy();
    return 1;
}

// returns queueSize if its an integer, otherwise -1
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

int pipeline_init(char *pluginNamesRaw[], int queueSize)
{
    if (g_pluginCount <= 0)
    {
        fprintf(stderr, "error");
        printf("Needs at least one plugin to start pipeline");
        return -1;
    }

    plugin_handles = malloc(g_pluginCount * sizeof(plugin_handle_t));
    if (!plugin_handles)
    {
        fprintf(stderr, "[ERROR] Allocating space for plugin_handles failed");
        return -1;
    }

    char **pluginNames = transformPluginName(pluginNamesRaw, g_pluginCount);

    for (int i = 0; i < g_pluginCount; i++)
    {
        plugin_handles[i].handle = dlopen(pluginNames[i], RTLD_NOW | RTLD_LOCAL);
        if (!plugin_handles[i].handle)
        {
            fprintf(stderr, "[ERROR] Failed to load plugin %s: %s\n", pluginNames[i], dlerror());
            for (int j = 0; j < i; j++)
            {
                dlclose(plugin_handles[j].handle);
                free(plugin_handles[j].name);
            }

            // Free transformed names
            for (int j = 0; j < g_pluginCount; j++)
            {
                free(pluginNames[j]);
            }
            free(pluginNames);
            free(plugin_handles);
            return -1;
        }

        plugin_handles[i].name = malloc(strlen(pluginNamesRaw[i]) + 1);
        strcpy(plugin_handles[i].name, pluginNamesRaw[i]);

        // Load the pointers to the functions from the handle

        plugin_handles[i].init = dlsym(plugin_handles[i].handle, "plugin_init");
        plugin_handles[i].fini = dlsym(plugin_handles[i].handle, "plugin_fini");
        plugin_handles[i].place_work = dlsym(plugin_handles[i].handle, "plugin_place_work");
        plugin_handles[i].attach = dlsym(plugin_handles[i].handle, "plugin_attach");
        plugin_handles[i].wait_finished = dlsym(plugin_handles[i].handle, "plugin_wait_finished");

        if (!plugin_handles[i].init || !plugin_handles[i].fini || !plugin_handles[i].place_work || !plugin_handles[i].attach || !plugin_handles[i].wait_finished)
        {
            fprintf(stderr, "[ERROR] Plugin %s missing required functions\n", pluginNamesRaw[i]);
            dlclose(plugin_handles[i].handle);
            free(plugin_handles[i].name);
            for (int j = 0; j < i; j++)
            {
                dlclose(plugin_handles[j].handle);
                free(plugin_handles[j].name);
            }
            for (int j = 0; j < g_pluginCount; j++)
            {
                free(pluginNames[j]);
            }
            free(pluginNames);
            free(plugin_handles);
            return -1;
        }
    }

    for (int i = 0; i < g_pluginCount - 1; i++)
    {
        plugin_handles[i].attach(plugin_handles[i + 1].place_work);
    }

    for (int i = 0; i < g_pluginCount; i++)
    {
        free(pluginNames[i]);
    }
    free(pluginNames);

    return 0;
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

int pipeline_destroy(void)
{
    if (!plugin_handles)
    {
        return 0;
    }

    for (int i = 0; i < g_pluginCount; i++)
    {
        if (plugin_handles[i].fini)
        {
            plugin_handles[i].fini();
        }
        if (plugin_handles[i].handle)
        {
            dlclose(plugin_handles[i].handle);
        }
        if (plugin_handles[i].name)
        {
            free(plugin_handles[i].name);
        }

        free(plugin_handles);
        plugin_handles = NULL;
        g_pluginCount = 0;
    }
    return 0;
}
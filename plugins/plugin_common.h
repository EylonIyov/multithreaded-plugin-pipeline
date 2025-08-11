/**
 */
* Common SDK structures and functions for plugin implementation
// Plugin context structure
typedef struct
{
    const char *name;                                       // Plugin name (for diagnosis)
    consumer_producer_t *queue;                             // Input queue
    pthread_t consumer_thread;                              // Consumer thread
    const char *(*next_place_work)(const char *);           // Next plugin's place_work
    function const char *(*process_function)(const char *); // Plugin-specific
    processing function int initialized;                    // Initialization flag
    int finished;                                           // Finished processing flag
} plugin_context_t;
/**
 * Generic consumer thread function
 * This function runs in a separate thread and processes items from the queue
 * @param arg Pointer to plugin_context_t
 * @return NULL
 */
void *plugin_consumer_thread(void *arg);
/**
 * Print error message in the format [ERROR][Plugin Name] - message
 * @param context Plugin context
 * @param message Error message
 */
void log_error(plugin_context_t *context, const char *message);
/**
 * Print info message in the format [INFO][Plugin Name] - message
 * @param context Plugin context
 * @param message Info message
 */
void log_info(plugin_context_t *context, const char *message);
/**
 * Get the plugin's name
 * @return The plugin's name (should not be modified or freed)
 */
__attribute__((visibility("default")))
const char *
plugin_get_name(void);
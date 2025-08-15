#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <assert.h>
#include "consumer_producer.h"

#define TEST_CAPACITY 5
#define NUM_PRODUCERS 2
#define NUM_CONSUMERS 2
#define ITEMS_PER_PRODUCER 10

// Test data structure for threading tests
typedef struct
{
    consumer_producer_t *queue;
    int thread_id;
    int items_produced;
    int items_consumed;
} thread_data_t;

// Test counters
static int total_produced = 0;
static int total_consumed = 0;
static pthread_mutex_t counter_lock = PTHREAD_MUTEX_INITIALIZER;

// Test functions
void test_init_destroy();
void test_basic_operations();
void test_blocking_behavior();
void test_threading();
void test_finish_signaling();
void run_all_tests();

// Helper functions
void *producer_thread(void *arg);
void *consumer_thread(void *arg);
char *create_test_item(int producer_id, int item_num);

int main()
{
    printf("=== Consumer-Producer Queue Test Suite ===\n\n");
    run_all_tests();
    printf("=== All tests completed ===\n");
    return 0;
}

void run_all_tests()
{
    test_init_destroy();
    test_basic_operations();
    test_blocking_behavior();
    test_threading();
    test_finish_signaling();
}

void test_init_destroy()
{
    printf("Test 1: Initialization and Destruction\n");

    consumer_producer_t queue;

    // Test successful initialization
    const char *error = consumer_producer_init(&queue, TEST_CAPACITY);
    assert(error == NULL);
    printf("  ✓ Queue initialization successful\n");

    // Test invalid capacity
    consumer_producer_t invalid_queue;
    error = consumer_producer_init(&invalid_queue, 0);
    assert(error != NULL);
    printf("  ✓ Invalid capacity properly rejected: %s\n", error);

    error = consumer_producer_init(&invalid_queue, -1);
    assert(error != NULL);
    printf("  ✓ Negative capacity properly rejected: %s\n", error);

    // Test NULL queue
    error = consumer_producer_init(NULL, TEST_CAPACITY);
    assert(error != NULL);
    printf("  ✓ NULL queue properly rejected: %s\n", error);

    // Clean up
    consumer_producer_destroy(&queue);
    printf("  ✓ Queue destruction completed\n");

    printf("Test 1: PASSED\n\n");
}

void test_basic_operations()
{
    printf("Test 2: Basic Put/Get Operations\n");

    consumer_producer_t queue;
    const char *error = consumer_producer_init(&queue, TEST_CAPACITY);
    assert(error == NULL);

    // Test putting items
    const char *test_items[] = {"item1", "item2", "item3"};
    for (int i = 0; i < 3; i++)
    {
        error = consumer_producer_put(&queue, test_items[i]);
        assert(error == NULL);
        printf("  ✓ Put item: %s\n", test_items[i]);
    }

    // Test getting items (FIFO order)
    for (int i = 0; i < 3; i++)
    {
        char *item = consumer_producer_get(&queue);
        assert(item != NULL);
        assert(strcmp(item, test_items[i]) == 0);
        printf("  ✓ Got item: %s\n", item);
        free(item);
    }

    // Test NULL item put
    error = consumer_producer_put(&queue, NULL);
    assert(error != NULL);
    printf("  ✓ NULL item properly rejected: %s\n", error);

    // Test NULL queue operations
    error = consumer_producer_put(NULL, "test");
    assert(error != NULL);
    printf("  ✓ NULL queue in put properly rejected: %s\n", error);

    char *item = consumer_producer_get(NULL);
    assert(item != NULL); // Should return error message, not NULL
    printf("  ✓ NULL queue in get properly handled: %s\n", item);

    consumer_producer_destroy(&queue);
    printf("Test 2: PASSED\n\n");
}

void test_blocking_behavior()
{
    printf("Test 3: Blocking Behavior\n");

    consumer_producer_t queue;
    const char *error = consumer_producer_init(&queue, 2); // Small capacity
    assert(error == NULL);

    // Fill the queue to capacity
    error = consumer_producer_put(&queue, "item1");
    assert(error == NULL);
    error = consumer_producer_put(&queue, "item2");
    assert(error == NULL);
    printf("  ✓ Queue filled to capacity (2/2)\n");

    // Empty the queue
    char *item1 = consumer_producer_get(&queue);
    char *item2 = consumer_producer_get(&queue);
    assert(item1 != NULL && item2 != NULL);
    printf("  ✓ Retrieved items: %s, %s\n", item1, item2);
    free(item1);
    free(item2);
    printf("  ✓ Queue emptied\n");

    consumer_producer_destroy(&queue);
    printf("Test 3: PASSED\n\n");
}

void test_threading()
{
    printf("Test 4: Multi-threaded Producer-Consumer\n");

    consumer_producer_t queue;
    const char *error = consumer_producer_init(&queue, TEST_CAPACITY);
    assert(error == NULL);

    // Reset counters
    pthread_mutex_lock(&counter_lock);
    total_produced = 0;
    total_consumed = 0;
    pthread_mutex_unlock(&counter_lock);

    // Create thread data
    thread_data_t producer_data[NUM_PRODUCERS];
    thread_data_t consumer_data[NUM_CONSUMERS];
    pthread_t producer_threads[NUM_PRODUCERS];
    pthread_t consumer_threads[NUM_CONSUMERS];

    // Initialize thread data
    for (int i = 0; i < NUM_PRODUCERS; i++)
    {
        producer_data[i].queue = &queue;
        producer_data[i].thread_id = i;
        producer_data[i].items_produced = 0;
    }

    for (int i = 0; i < NUM_CONSUMERS; i++)
    {
        consumer_data[i].queue = &queue;
        consumer_data[i].thread_id = i;
        consumer_data[i].items_consumed = 0;
    }

    // Start producer threads
    for (int i = 0; i < NUM_PRODUCERS; i++)
    {
        pthread_create(&producer_threads[i], NULL, producer_thread, &producer_data[i]);
    }

    // Start consumer threads
    for (int i = 0; i < NUM_CONSUMERS; i++)
    {
        pthread_create(&consumer_threads[i], NULL, consumer_thread, &consumer_data[i]);
    }

    // Wait for producers to finish
    for (int i = 0; i < NUM_PRODUCERS; i++)
    {
        pthread_join(producer_threads[i], NULL);
        printf("  ✓ Producer %d finished (%d items)\n", i, producer_data[i].items_produced);
    }

    // Signal that production is finished
    consumer_producer_signal_finished(&queue);
    printf("  ✓ Finished signal sent\n");

    // Wait for consumers to finish
    for (int i = 0; i < NUM_CONSUMERS; i++)
    {
        pthread_join(consumer_threads[i], NULL);
        printf("  ✓ Consumer %d finished (%d items)\n", i, consumer_data[i].items_consumed);
    }

    printf("  Total produced: %d, Total consumed: %d\n", total_produced, total_consumed);
    assert(total_produced == NUM_PRODUCERS * ITEMS_PER_PRODUCER);
    assert(total_consumed == total_produced);

    consumer_producer_destroy(&queue);
    printf("Test 4: PASSED\n\n");
}

void test_finish_signaling()
{
    printf("Test 5: Finish Signaling\n");

    consumer_producer_t queue;
    const char *error = consumer_producer_init(&queue, TEST_CAPACITY);
    assert(error == NULL);

    // Test signaling finished
    consumer_producer_signal_finished(&queue);
    printf("  ✓ Finish signal sent\n");

    // Test waiting for finished signal
    int result = consumer_producer_wait_finished(&queue);
    assert(result == 0);
    printf("  ✓ Finish signal received\n");

    // Test with NULL queue
    result = consumer_producer_wait_finished(NULL);
    assert(result == -1);
    printf("  ✓ NULL queue properly handled\n");

    consumer_producer_destroy(&queue);
    printf("Test 5: PASSED\n\n");
}

void *producer_thread(void *arg)
{
    thread_data_t *data = (thread_data_t *)arg;

    for (int i = 0; i < ITEMS_PER_PRODUCER; i++)
    {
        char *item = create_test_item(data->thread_id, i);
        const char *error = consumer_producer_put(data->queue, item);

        if (error == NULL)
        {
            data->items_produced++;
            pthread_mutex_lock(&counter_lock);
            total_produced++;
            pthread_mutex_unlock(&counter_lock);
        }
        else
        {
            printf("  Producer %d failed to put item: %s\n", data->thread_id, error);
        }

        free(item);
        usleep(1000); // Small delay to simulate work
    }

    return NULL;
}

void *consumer_thread(void *arg)
{
    thread_data_t *data = (thread_data_t *)arg;

    while (1)
    {
        char *item = consumer_producer_get(data->queue);
        if (item == NULL)
        {
            // Queue is finished
            break;
        }

        // Check if this is an error message (consumer_producer_get returns error messages for NULL queue)
        if (strstr(item, "Error:") != NULL)
        {
            free(item);
            break;
        }

        data->items_consumed++;
        pthread_mutex_lock(&counter_lock);
        total_consumed++;
        pthread_mutex_unlock(&counter_lock);

        free(item);
        usleep(1500); // Small delay to simulate processing
    }

    return NULL;
}

char *create_test_item(int producer_id, int item_num)
{
    char *item = malloc(64);
    snprintf(item, 64, "P%d-Item%d", producer_id, item_num);
    return item;
}
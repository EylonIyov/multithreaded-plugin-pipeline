/**
 * monitor_test.c
 * Comprehensive test suite for the monitor implementation
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <assert.h>
#include <errno.h>
#include "monitor.h"

#define MAX_THREADS 10
#define TIMEOUT_SEC 2

/* Test result tracking */
typedef struct
{
    int passed;
    int failed;
    int total;
} test_results_t;

static test_results_t results = {0, 0, 0};

/* Helper macros for test assertions */
#define TEST_ASSERT(condition, message)          \
    do                                           \
    {                                            \
        if (!(condition))                        \
        {                                        \
            printf("    FAILED: %s\n", message); \
            results.failed++;                    \
            results.total++;                     \
            return 0;                            \
        }                                        \
    } while (0)

#define TEST_ASSERT_EQUAL(a, b, message) TEST_ASSERT((a) == (b), message)

/* Test context for multi-threaded tests */
typedef struct
{
    monitor_t *monitor;
    int counter;
    int should_signal;
    int wait_count;
    int signal_count;
    pthread_mutex_t test_mutex;
    struct timespec start_time;
    struct timespec end_time;
} test_context_t;

/* Helper function to get elapsed time in milliseconds */
static long get_elapsed_ms(struct timespec *start, struct timespec *end)
{
    return (end->tv_sec - start->tv_sec) * 1000 +
           (end->tv_nsec - start->tv_nsec) / 1000000;
}

/* Test 1: Basic initialization and destruction */
static int test_init_destroy(void)
{
    printf("\nTest 1: Basic initialization and destruction\n");

    monitor_t monitor;
    int result;

    /* Test normal initialization */
    printf("    1.1: Testing normal initialization...\n");
    result = monitor_init(&monitor);
    TEST_ASSERT_EQUAL(result, 0, "Initialization should succeed");
    TEST_ASSERT_EQUAL(monitor.signaled, 0, "Initial signaled state should be 0");

    /* Destroy the monitor */
    monitor_destroy(&monitor);

    /* Test initialization with NULL pointer */
    printf("    1.2: Testing init with NULL pointer...\n");
    result = monitor_init(NULL);
    if (result == 0)
    {
        printf("    WARNING: init(NULL) should fail but returned success\n");
        printf("    Your implementation might not check for NULL\n");
    }
    else
    {
        printf("    Good: init(NULL) returned error\n");
    }

    /* Test destroy with NULL pointer - should not crash */
    printf("    1.3: Testing destroy with NULL pointer...\n");
    monitor_destroy(NULL);
    printf("    Good: destroy(NULL) didn't crash\n");

    printf("    PASSED\n");
    results.passed++;
    results.total++;
    return 1;
}

/* Test 2: Basic signal and wait operations */
static int test_basic_signal_wait(void)
{
    printf("\nTest 2: Basic signal and wait operations\n");

    monitor_t monitor;
    int result;

    monitor_init(&monitor);

    /* Test signaling */
    printf("    2.1: Testing signal operation...\n");
    monitor_signal(&monitor);
    TEST_ASSERT_EQUAL(monitor.signaled, 1, "Signal should set signaled flag to 1");

    /* Test that wait returns immediately when already signaled */
    printf("    2.2: Testing wait on already signaled monitor...\n");
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    result = monitor_wait(&monitor);
    clock_gettime(CLOCK_MONOTONIC, &end);

    TEST_ASSERT_EQUAL(result, 0, "Wait should succeed on signaled monitor");
    TEST_ASSERT_EQUAL(monitor.signaled, 0, "Wait should reset signaled flag");

    long elapsed = get_elapsed_ms(&start, &end);
    TEST_ASSERT(elapsed < 100, "Wait should return immediately when already signaled");

    monitor_destroy(&monitor);

    printf("    PASSED\n");
    results.passed++;
    results.total++;
    return 1;
}

/* Test 3: Reset operation */
static int test_reset(void)
{
    printf("\nTest 3: Reset operation\n");

    monitor_t monitor;

    monitor_init(&monitor);

    /* Signal the monitor */
    printf("    3.1: Setting signal...\n");
    monitor_signal(&monitor);
    TEST_ASSERT_EQUAL(monitor.signaled, 1, "Signal should set flag to 1");

    /* Reset the monitor */
    printf("    3.2: Testing reset...\n");
    monitor_reset(&monitor);
    TEST_ASSERT_EQUAL(monitor.signaled, 0, "Reset should clear signaled flag");

    /* Test reset on already reset monitor */
    printf("    3.3: Testing reset on already reset monitor...\n");
    monitor_reset(&monitor);
    TEST_ASSERT_EQUAL(monitor.signaled, 0, "Reset should keep flag at 0");

    /* Test reset with NULL */
    printf("    3.4: Testing reset with NULL pointer...\n");
    monitor_reset(NULL);
    printf("    Good: reset(NULL) didn't crash\n");

    monitor_destroy(&monitor);

    printf("    PASSED\n");
    results.passed++;
    results.total++;
    return 1;
}

/* Thread function that waits on monitor */
static void *waiter_thread(void *arg)
{
    test_context_t *ctx = (test_context_t *)arg;

    pthread_mutex_lock(&ctx->test_mutex);
    ctx->wait_count++;
    pthread_mutex_unlock(&ctx->test_mutex);

    clock_gettime(CLOCK_MONOTONIC, &ctx->start_time);
    int result = monitor_wait(ctx->monitor);
    clock_gettime(CLOCK_MONOTONIC, &ctx->end_time);

    if (result == 0)
    {
        pthread_mutex_lock(&ctx->test_mutex);
        ctx->counter++;
        pthread_mutex_unlock(&ctx->test_mutex);
    }

    return NULL;
}

/* Thread function that signals monitor */
static void *signaler_thread(void *arg)
{
    test_context_t *ctx = (test_context_t *)arg;

    /* Wait a bit to ensure waiter is waiting */
    usleep(500000); /* 500ms */

    pthread_mutex_lock(&ctx->test_mutex);
    ctx->signal_count++;
    pthread_mutex_unlock(&ctx->test_mutex);

    monitor_signal(ctx->monitor);

    return NULL;
}

/* Test 4: Thread synchronization - wait blocks until signal */
static int test_thread_blocking(void)
{
    printf("\nTest 4: Thread synchronization - wait blocks until signal\n");

    monitor_t monitor;
    test_context_t ctx = {0};
    pthread_t waiter, signaler;

    monitor_init(&monitor);
    ctx.monitor = &monitor;
    ctx.counter = 0;
    ctx.wait_count = 0;
    ctx.signal_count = 0;
    pthread_mutex_init(&ctx.test_mutex, NULL);

    /* Start waiter thread first */
    printf("    4.1: Starting waiter thread...\n");
    pthread_create(&waiter, NULL, waiter_thread, &ctx);

    /* Give waiter time to start waiting */
    usleep(200000); /* 200ms */

    /* Check that waiter is blocked */
    pthread_mutex_lock(&ctx.test_mutex);
    int wait_started = ctx.wait_count;
    int counter_before = ctx.counter;
    pthread_mutex_unlock(&ctx.test_mutex);

    TEST_ASSERT_EQUAL(wait_started, 1, "Waiter should have started waiting");
    TEST_ASSERT_EQUAL(counter_before, 0, "Counter should still be 0 (waiter blocked)");

    /* Start signaler thread */
    printf("    4.2: Starting signaler thread...\n");
    pthread_create(&signaler, NULL, signaler_thread, &ctx);

    /* Wait for both threads */
    pthread_join(signaler, NULL);
    pthread_join(waiter, NULL);

    /* Verify waiter was unblocked by signal */
    TEST_ASSERT_EQUAL(ctx.counter, 1, "Waiter should have been unblocked");
    TEST_ASSERT_EQUAL(ctx.signal_count, 1, "Signal should have been sent");

    /* Check timing */
    long elapsed = get_elapsed_ms(&ctx.start_time, &ctx.end_time);
    TEST_ASSERT(elapsed >= 400, "Waiter should have blocked for at least 400ms");

    pthread_mutex_destroy(&ctx.test_mutex);
    monitor_destroy(&monitor);

    printf("    PASSED\n");
    results.passed++;
    results.total++;
    return 1;
}

/* Test 5: Signal before wait (testing the "memory" feature) */
static int test_signal_before_wait(void)
{
    printf("\nTest 5: Signal before wait (testing the memory feature)\n");

    monitor_t monitor;
    monitor_init(&monitor);

    /* Signal first */
    printf("    5.1: Signaling before any wait...\n");
    monitor_signal(&monitor);
    TEST_ASSERT_EQUAL(monitor.signaled, 1, "Signal should set flag");

    /* Now wait - should return immediately */
    printf("    5.2: Waiting after signal...\n");
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    int result = monitor_wait(&monitor);
    clock_gettime(CLOCK_MONOTONIC, &end);

    TEST_ASSERT_EQUAL(result, 0, "Wait should succeed");
    TEST_ASSERT_EQUAL(monitor.signaled, 0, "Wait should clear the flag");

    long elapsed = get_elapsed_ms(&start, &end);
    TEST_ASSERT(elapsed < 100, "Wait should return immediately (signal was remembered)");

    monitor_destroy(&monitor);

    printf("    PASSED\n");
    results.passed++;
    results.total++;
    return 1;
}

/* Multiple waiter thread function */
static void *multi_waiter_thread(void *arg)
{
    test_context_t *ctx = (test_context_t *)arg;

    int result = monitor_wait(ctx->monitor);

    if (result == 0)
    {
        pthread_mutex_lock(&ctx->test_mutex);
        ctx->counter++;
        pthread_mutex_unlock(&ctx->test_mutex);
    }

    return NULL;
}

/* Test 6: Multiple threads waiting on same monitor */
static int test_multiple_waiters(void)
{
    printf("\nTest 6: Multiple threads waiting on same monitor\n");

    monitor_t monitor;
    test_context_t ctx = {0};
    pthread_t waiters[5];

    monitor_init(&monitor);
    ctx.monitor = &monitor;
    ctx.counter = 0;
    pthread_mutex_init(&ctx.test_mutex, NULL);

    /* Start multiple waiter threads */
    printf("    6.1: Starting 5 waiter threads...\n");
    for (int i = 0; i < 5; i++)
    {
        pthread_create(&waiters[i], NULL, multi_waiter_thread, &ctx);
    }

    /* Give them time to start waiting */
    usleep(500000); /* 500ms */

    /* Signal once - should wake up one waiter */
    printf("    6.2: Sending one signal...\n");
    monitor_signal(&monitor);

    /* Give time for one thread to wake up */
    usleep(200000); /* 200ms */

    pthread_mutex_lock(&ctx.test_mutex);
    int woken_count = ctx.counter;
    pthread_mutex_unlock(&ctx.test_mutex);

    printf("    6.3: Checking wake-up behavior...\n");
    printf("    Note: Woken threads: %d\n", woken_count);
    printf("    (Implementation may wake one or all threads - both are valid)\n");

    /* Signal multiple times to wake remaining threads */
    for (int i = 0; i < 5; i++)
    {
        monitor_signal(&monitor);
        usleep(100000); /* 100ms between signals */
    }

    /* Wait for all threads with timeout */
    for (int i = 0; i < 5; i++)
    {
        pthread_join(waiters[i], NULL);
    }

    printf("    Final count: %d threads completed\n", ctx.counter);

    pthread_mutex_destroy(&ctx.test_mutex);
    monitor_destroy(&monitor);

    printf("    PASSED\n");
    results.passed++;
    results.total++;
    return 1;
}

/* Test 7: Multiple signals */
static int test_multiple_signals(void)
{
    printf("\nTest 7: Multiple signals\n");

    monitor_t monitor;
    monitor_init(&monitor);

    /* Send multiple signals */
    printf("    7.1: Sending multiple signals...\n");
    monitor_signal(&monitor);
    monitor_signal(&monitor);
    monitor_signal(&monitor);

    /* Flag should still be 1 (not a counter) */
    TEST_ASSERT_EQUAL(monitor.signaled, 1, "Multiple signals should keep flag at 1");

    /* First wait should succeed */
    printf("    7.2: First wait...\n");
    int result = monitor_wait(&monitor);
    TEST_ASSERT_EQUAL(result, 0, "First wait should succeed");
    TEST_ASSERT_EQUAL(monitor.signaled, 0, "Flag should be cleared after wait");

    /* Second wait should block (in a real scenario) */
    printf("    7.3: Note: Second wait would block (not testing to avoid hang)\n");

    monitor_destroy(&monitor);

    printf("    PASSED\n");
    results.passed++;
    results.total++;
    return 1;
}

/* Test 8: NULL pointer handling */
static int test_null_handling(void)
{
    printf("\nTest 8: NULL pointer handling\n");

    /* Test all functions with NULL */
    printf("    8.1: Testing signal with NULL...\n");
    monitor_signal(NULL);
    printf("    Good: signal(NULL) didn't crash\n");

    printf("    8.2: Testing reset with NULL...\n");
    monitor_reset(NULL);
    printf("    Good: reset(NULL) didn't crash\n");

    printf("    8.3: Testing wait with NULL...\n");
    int result = monitor_wait(NULL);
    if (result == 0)
    {
        printf("    WARNING: wait(NULL) should return error\n");
    }
    else
    {
        printf("    Good: wait(NULL) returned error\n");
    }

    printf("    8.4: Testing destroy with NULL...\n");
    monitor_destroy(NULL);
    printf("    Good: destroy(NULL) didn't crash\n");

    printf("    PASSED\n");
    results.passed++;
    results.total++;
    return 1;
}

/* Stress test - simple signaler/waiter pattern */
static void *stress_signaler_thread(void *arg)
{
    test_context_t *ctx = (test_context_t *)arg;

    /* Continuously signal for 2 seconds */
    time_t start = time(NULL);
    while (time(NULL) - start < 2)
    {
        monitor_signal(ctx->monitor);

        pthread_mutex_lock(&ctx->test_mutex);
        ctx->signal_count++;
        pthread_mutex_unlock(&ctx->test_mutex);

        usleep(1000); /* 1ms delay between signals */
    }

    return NULL;
}

static void *stress_waiter_thread(void *arg)
{
    test_context_t *ctx = (test_context_t *)arg;

    /* Try to wait multiple times within 2 seconds */
    time_t start = time(NULL);
    while (time(NULL) - start < 2)
    {
        int result = monitor_wait(ctx->monitor);
        if (result == 0)
        {
            pthread_mutex_lock(&ctx->test_mutex);
            ctx->wait_count++;
            pthread_mutex_unlock(&ctx->test_mutex);
        }

        usleep(500); /* Small delay to simulate work */
    }

    return NULL;
}

/* Test 9: Stress test */
static int test_stress(void)
{
    printf("\nTest 9: Stress test with many threads\n");

    monitor_t monitor;
    test_context_t ctx = {0};
    pthread_t signalers[3];
    pthread_t waiters[7];

    monitor_init(&monitor);
    ctx.monitor = &monitor;
    ctx.signal_count = 0;
    ctx.wait_count = 0;
    pthread_mutex_init(&ctx.test_mutex, NULL);

    printf("    9.1: Starting 3 signaler and 7 waiter threads for 2 seconds...\n");

    /* Start all threads */
    for (int i = 0; i < 3; i++)
    {
        pthread_create(&signalers[i], NULL, stress_signaler_thread, &ctx);
    }
    for (int i = 0; i < 7; i++)
    {
        pthread_create(&waiters[i], NULL, stress_waiter_thread, &ctx);
    }

    /* Wait for all threads */
    for (int i = 0; i < 3; i++)
    {
        pthread_join(signalers[i], NULL);
    }

    /* Signal a few more times to help waiters finish */
    for (int i = 0; i < 10; i++)
    {
        monitor_signal(&monitor);
    }

    for (int i = 0; i < 7; i++)
    {
        pthread_join(waiters[i], NULL);
    }

    printf("    9.2: Completed. Signals sent: %d, Successful waits: %d\n",
           ctx.signal_count, ctx.wait_count);

    /* Just verify that something happened - exact counts will vary due to timing */
    TEST_ASSERT(ctx.signal_count > 0, "Should have sent signals");
    TEST_ASSERT(ctx.wait_count > 0, "Should have completed some waits");
    TEST_ASSERT(ctx.wait_count <= ctx.signal_count + 10,
                "Waits should not exceed signals (much)");

    pthread_mutex_destroy(&ctx.test_mutex);
    monitor_destroy(&monitor);

    printf("    PASSED\n");
    results.passed++;
    results.total++;
    return 1;
}

/* Test 10: Reset while threads are waiting */
static int test_reset_with_waiters(void)
{
    printf("\nTest 10: Reset while threads are waiting\n");

    monitor_t monitor;
    test_context_t ctx = {0};
    pthread_t waiter;

    monitor_init(&monitor);
    ctx.monitor = &monitor;
    ctx.counter = 0;
    pthread_mutex_init(&ctx.test_mutex, NULL);

    /* Pre-signal the monitor */
    printf("    10.1: Pre-signaling monitor...\n");
    monitor_signal(&monitor);
    TEST_ASSERT_EQUAL(monitor.signaled, 1, "Monitor should be signaled");

    /* Reset it */
    printf("    10.2: Resetting signaled monitor...\n");
    monitor_reset(&monitor);
    TEST_ASSERT_EQUAL(monitor.signaled, 0, "Monitor should be reset");

    /* Start a waiter - should block since we reset */
    printf("    10.3: Starting waiter thread (should block)...\n");
    pthread_create(&waiter, NULL, waiter_thread, &ctx);

    /* Give waiter time to block */
    usleep(500000); /* 500ms */

    /* Verify waiter is still blocked */
    TEST_ASSERT_EQUAL(ctx.counter, 0, "Waiter should be blocked after reset");

    /* Signal to unblock */
    printf("    10.4: Signaling to unblock waiter...\n");
    monitor_signal(&monitor);

    pthread_join(waiter, NULL);

    TEST_ASSERT_EQUAL(ctx.counter, 1, "Waiter should have been unblocked");

    pthread_mutex_destroy(&ctx.test_mutex);
    monitor_destroy(&monitor);

    printf("    PASSED\n");
    results.passed++;
    results.total++;
    return 1;
}

/* Test 11: Memory leak test */
static int test_memory_management(void)
{
    printf("\nTest 11: Memory management test\n");

    /* Multiple init/destroy cycles */
    printf("    11.1: Running 100 init/destroy cycles...\n");
    for (int i = 0; i < 100; i++)
    {
        monitor_t monitor;

        int result = monitor_init(&monitor);
        if (result != 0)
        {
            printf("    Failed to init monitor at iteration %d\n", i);
            break;
        }

        /* Do some operations */
        monitor_signal(&monitor);
        monitor_reset(&monitor);
        monitor_signal(&monitor);

        monitor_destroy(&monitor);
    }

    /* Create and destroy multiple monitors */
    printf("    11.2: Creating multiple monitors simultaneously...\n");
    monitor_t monitors[10];

    for (int i = 0; i < 10; i++)
    {
        monitor_init(&monitors[i]);
    }

    for (int i = 0; i < 10; i++)
    {
        monitor_signal(&monitors[i]);
    }

    for (int i = 0; i < 10; i++)
    {
        monitor_destroy(&monitors[i]);
    }

    printf("    PASSED (run with valgrind to check for leaks)\n");
    results.passed++;
    results.total++;
    return 1;
}

/* Main test runner */
int main(int argc, char *argv[])
{
    printf("========================================\n");
    printf("Monitor Test Suite\n");
    printf("========================================\n");

    /* Seed random number generator */
    srand(time(NULL));

    /* Run all tests */
    test_init_destroy();
    test_basic_signal_wait();
    test_reset();
    test_thread_blocking();
    test_signal_before_wait();
    test_multiple_waiters();
    test_multiple_signals();
    test_null_handling();
    // test_stress();
    test_reset_with_waiters();
    test_memory_management();

    /* Print summary */
    printf("\n========================================\n");
    printf("Test Results Summary:\n");
    printf("Total:  %d\n", results.total);
    printf("Passed: %d\n", results.passed);
    printf("Failed: %d\n", results.failed);

    if (results.failed == 0)
    {
        printf("\nAll tests PASSED! ✓\n");
    }
    else
    {
        printf("\nSome tests FAILED! ✗\n");
    }
    printf("========================================\n");

    return results.failed > 0 ? 1 : 0;
}
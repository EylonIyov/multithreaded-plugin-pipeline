#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include "monitor.h"

#define SIZE 5

typedef struct
{
    monitor_t *monitor;
    int thread_id;
} thread_data_t;

void *worker_thread(void *arg)
{
    thread_data_t *data = (thread_data_t *)arg;

    printf("Thread %d: Waiting on monitor...\n", data->thread_id);
    monitor_wait(data->monitor);
    printf("Thread %d: Woke up!\n", data->thread_id);

    return NULL;
}

void test_multiple_threads()
{

    monitor_t monitor;
    monitor_init(&monitor);

    pthread_t threads[SIZE];
    thread_data_t thread_data[SIZE];

    for (int i = 0; i < SIZE; i++)
    {

        thread_data[i].monitor = &monitor;
        thread_data[i].thread_id = i;
        pthread_create(&threads[i], NULL, worker_thread, &thread_data[i]);
    }
    sleep(1);

    printf("Main: Signaling monitor\n");

    for (int i = 0; i < SIZE; i++)
    {
        monitor_signal(&monitor);
    }

    for (int i = 0; i < SIZE; i++)
    {
        pthread_join(threads[i], NULL);
    }

    monitor_destroy(&monitor);
}

// Test 2

void *late_waiter(void *arg)
{
    monitor_t *mon = (monitor_t *)arg;

    sleep(2); // Arrive AFTER signal
    printf("Late thread: Starting wait\n");
    monitor_wait(mon);
    printf("Late thread: Exited immediately! (signal was sticky)\n");

    return NULL;
}

void test_manual_reset()
{
    monitor_t monitor;
    monitor_init(&monitor);

    // Signal BEFORE any thread waits
    printf("Signaling with no waiters\n");
    monitor_signal(&monitor);

    // Create thread that starts waiting AFTER signal
    pthread_t thread;
    pthread_create(&thread, NULL, late_waiter, &monitor);

    pthread_join(thread, NULL);

    // Now test reset
    monitor_reset(&monitor);

    // This thread should now block
    pthread_create(&thread, NULL, worker_thread, &monitor);
    sleep(1);
    printf("Thread should be blocked after reset\n");

    monitor_signal(&monitor);
    pthread_join(thread, NULL);

    monitor_destroy(&monitor);
}

// Test 3:
void test_multiple_waiters()
{
    monitor_t monitor;
    monitor_init(&monitor);

    pthread_t threads[5];
    thread_data_t data[5];

    // Create 5 waiting threads
    for (int i = 0; i < 5; i++)
    {
        data[i].monitor = &monitor;
        data[i].thread_id = i;
        pthread_create(&threads[i], NULL, worker_thread, &data[i]);
    }

    sleep(1); // Let all threads start waiting

    printf("Main: Signaling once\n");
    monitor_signal(&monitor);

    sleep(2); // Give time for threads to wake

    // With pthread_cond_signal: only 1 thread wakes
    // With pthread_cond_broadcast: all 5 threads wake
    // For manual-reset, you probably want broadcast

    monitor_reset(&monitor);  // Clean up for any still waiting
    monitor_signal(&monitor); // Wake remaining threads

    for (int i = 0; i < 5; i++)
    {
        pthread_join(threads[i], NULL);
    }

    monitor_destroy(&monitor);
}

// Test 4:

void *quick_signaler(void *arg)
{
    monitor_t *mon = (monitor_t *)arg;

    // Signal immediately (potential race!)
    monitor_signal(mon);
    printf("Quick signal sent!\n");

    return NULL;
}

void *slow_waiter(void *arg)
{
    monitor_t *mon = (monitor_t *)arg;

    usleep(100000); // Small delay to create race window
    printf("Slow waiter: Starting wait\n");
    monitor_wait(mon);
    printf("Slow waiter: Got signal (no race!)\n");

    return NULL;
}

void test_race_condition()
{
    monitor_t monitor;
    monitor_init(&monitor);

    pthread_t signaler, waiter;

    // Start both threads nearly simultaneously
    pthread_create(&waiter, NULL, slow_waiter, &monitor);
    pthread_create(&signaler, NULL, quick_signaler, &monitor);

    pthread_join(signaler, NULL);
    pthread_join(waiter, NULL);

    printf("Race condition test: PASSED\n");

    monitor_destroy(&monitor);
}

// Test 5

void *stress_worker(void *arg)
{
    monitor_t *mon = (monitor_t *)arg;

    for (int i = 0; i < 100; i++)
    {
        monitor_wait(mon);
        monitor_reset(mon); // Reset for next iteration
        usleep(1000);       // Small work
    }

    return NULL;
}

void test_stress()
{
    monitor_t monitor;
    monitor_init(&monitor);

    pthread_t threads[10];

    // Create many threads
    for (int i = 0; i < 10; i++)
    {
        pthread_create(&threads[i], NULL, stress_worker, &monitor);
    }

    // Signal many times
    for (int i = 0; i < 1000; i++)
    {
        usleep(5000);
        monitor_signal(&monitor);
    }

    // Clean shutdown
    for (int i = 0; i < 10; i++)
    {
        pthread_join(threads[i], NULL);
    }

    monitor_destroy(&monitor);
}

void main()
{
    printf("=== Monitor Test Suite ===\n\n");

    printf("Test 1: Basic Signal\n");
    test_multiple_threads();

    printf("\nTest 2: Manual Reset\n");
    test_manual_reset();

    printf("\nTest 3: Multiple Waiters\n");
    test_multiple_waiters();

    printf("\nTest 4: Race Condition\n");
    test_race_condition();

    printf("\nTest 5: Stress Test\n");
    test_stress();

    printf("\n=== All Tests Passed ===\n");
    return 0;
}
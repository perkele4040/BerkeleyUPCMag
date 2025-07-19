#include <upc.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <upc_tick.h>

#define N 100000 // Global size of array

// Shared array of integers
shared int arr[N];

void verify_sorted() {
    for (int i = 0; i < N - 1; i++) {
        if (arr[i] > arr[i + 1]) {
            printf("Array is not sorted at index %d: %d > %d\n", i, arr[i], arr[i + 1]);
            return;
        }
    }
    printf("Array is sorted correctly.\n");
}

void print_array() {
    printf("array: ");
    for (int i = 0; i < N; i++) {
        printf("%d ", arr[i]);
    }
    printf("\n");
}

void bubble_sort() {
    upc_tick_t time_start, time_end;
    double time_elapsed;
    int temp;
    int phase;
    upc_barrier;
    
    time_start = upc_ticks_now();
    for (phase = 0; phase < N; phase++) {
        for (int i = MYTHREAD; i < N / 2; i += THREADS) {
            int idx = 2 * i + (phase % 2);
            if (idx + 1 < N) {
                if (arr[idx] > arr[idx + 1]) {
                    // Swap
                    temp = arr[idx];
                    arr[idx] = arr[idx + 1];
                    arr[idx + 1] = temp;
                }
            }
        }
        upc_barrier;
    }
    time_end = upc_ticks_now();
    time_elapsed = upc_ticks_to_ns(time_end - time_start);

    if (MYTHREAD == 0) {
        verify_sorted();
        printf("\nElapsed time for main calculation in milliseconds:\n");
    }
    upc_barrier;
    printf("Thread %d - %f milliseconds\n", MYTHREAD, time_elapsed/1000000.0);

}

int main() {
    
    

    // Initialize array with random values on thread 0
    if (MYTHREAD == 0) {
        srand(time(NULL));
        for (int i = 0; i < N; i++) {
            arr[i] = rand() % 20000 - 10000; // Random values between -1000 and 1000
        }
    }
    upc_barrier; // Ensure all threads have initialized the array

    bubble_sort();

    return 0;
}

#include <upc.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <upc_tick.h>

#define ARRAY_SIZE 1000000
#define MAX_VAL 1000
#define MIN_VAL -1000

// Shared array accessible by all threads
shared int shared_array[ARRAY_SIZE];

// Helper swap function
void swap(int *a, int *b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

// Serial partition function used by each thread locally
int partition(int *arr, int low, int high) {
    int pivot = arr[high];
    int i = low - 1;
    for (int j = low; j <= high - 1; j++) {
        if (arr[j] <= pivot) {
            i++;
            swap(&arr[i], &arr[j]);
        }
    }
    swap(&arr[i + 1], &arr[high]);
    return (i + 1);
}

// Serial quicksort used locally
void quicksort(int *arr, int low, int high) {
    if (low < high) {
        int p = partition(arr, low, high);
        quicksort(arr, low, p - 1);
        quicksort(arr, p + 1, high);
    }
}

int main() {
    upc_tick_t time_start, time_end;
    double time_elapsed;
    int chunk_size = ARRAY_SIZE / THREADS;
    int start = MYTHREAD * chunk_size;
    int end = (MYTHREAD == THREADS - 1) ? ARRAY_SIZE : start + chunk_size;

    // Thread 0 initializes the array with random values
    if (MYTHREAD == 0) {
        srand(time(NULL));
        for (int i = 0; i < ARRAY_SIZE; i++) {
            shared_array[i] = (rand() % (MAX_VAL - MIN_VAL + 1)) + MIN_VAL;
        }
    }

    // Ensure all threads see the initialized data
    upc_barrier;

    // Copy local portion of the shared array to private memory
    int local_size = end - start;
    int *local_array = malloc(local_size * sizeof(int));
    for (int i = 0; i < local_size; i++) {
        local_array[i] = shared_array[start + i];
    }
    time_start = upc_ticks_now();
    // Perform local quicksort
    quicksort(local_array, 0, local_size - 1);
    time_end = upc_ticks_now();
    // how to make ths work without copying to local array?
    // quicksort(shared_array, start, end-1);

    // Copy sorted local array back to shared array
    for (int i = 0; i < local_size; i++) {
        shared_array[start + i] = local_array[i];
    }

    free(local_array);
    upc_barrier;
    time_elapsed = upc_ticks_to_ns(time_end - time_start);
    // Optional: Thread 0 gathers and prints the final array
    if (MYTHREAD == 0) {
        /*printf("Partially sorted array (locally sorted chunks):\n");
        for (int i = 0; i < ARRAY_SIZE; i++) {
            printf("%d ", shared_array[i]);
        }
        printf("\n");

        // Further global merge would be required for full sort
        printf("\nNote: This is only locally sorted; a full merge step is needed for global order.\n");*/
        printf("Elapsed time for main calculation in milliseconds:\n");
    }
    upc_barrier;
    printf("Thread %d - %f milliseconds\n", MYTHREAD, time_elapsed/1000000.0);

    return 0;
}

#include <upc.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <upc_tick.h>
#include <upc_strict.h>
#include <upc_collective.h>

#define N 10000 // Global size of array
#define LOWER_BOUND -1000
#define UPPER_BOUND 1000

// Shared array of integers
shared [] int array[N];
shared [N/THREADS] int local_array[N];
unsigned int * seed;

void verify_sorted() {
    int start = MYTHREAD * (N / THREADS);
    int end = start + N/THREADS;
    for (int i = start; i < end-1; i++) {
        if (local_array[i] > local_array[i + 1]) {
            printf("Subarray of thread %d is not sorted at index %d: %d > %d\n", MYTHREAD, i, local_array[i], local_array[i + 1]);
            return;
        }
    }
    printf("Subarray of thread %d is sorted correctly.\n", MYTHREAD);
}

void bubble_sort() {
    // Zmienne pomocnicze
    int temp, phase;
    int start = MYTHREAD * (N / THREADS);
    int end = start + N/THREADS;
    printf("Thread %d sorting from %d to %d\n", MYTHREAD, start, end);
    
    for (phase = 0; phase < N; phase++) {
        for (int i = start; i < end - 1; i++) {
            if (local_array[i] > local_array[i + 1]) {
                    // Swap
                temp = local_array[i];
                local_array[i] = local_array[i + 1];
                local_array[i + 1] = temp;
            }
        }
    }
        verify_sorted();
    

}

int main() {
    // Inicjalizacja ziarna losowego
    seed = (unsigned int *)malloc(sizeof(unsigned int));
    *seed = time(NULL)*1234 + MYTHREAD;
    srand(*seed);

    // Zmienne pomocnicze do pomiaru czasu
    upc_tick_t time_start, time_end;
    double time_elapsed;
    
    // Inicjalizacja tabeli początkowej
    if(MYTHREAD==0) {
        for(int i = 0; i < N; i++) 
            array[i] = LOWER_BOUND + (rand_r(seed) % (UPPER_BOUND - LOWER_BOUND + 1));

    }      
    upc_barrier;
    time_start = upc_ticks_now();
    upc_all_scatter(local_array, array, sizeof(int)*(N/THREADS), UPC_IN_NOSYNC | UPC_OUT_NOSYNC);
    upc_barrier;
    bubble_sort();
    upc_all_gather(array, local_array, sizeof(int)*(N/THREADS), UPC_IN_NOSYNC | UPC_OUT_NOSYNC);
    time_end = upc_ticks_now();
    time_elapsed = upc_ticks_to_ns(time_end - time_start);
    if (MYTHREAD == 0) {
        printf("\nElapsed time for main calculation in milliseconds:\n");
        fflush(stdout);
    }
    upc_barrier;
    printf("Thread %d - %f milliseconds\n", MYTHREAD, time_elapsed/1000000.0);
    return 0;
}

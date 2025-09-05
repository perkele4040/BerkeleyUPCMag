#include <upc.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <upc_tick.h>
#include <upc_collective.h>
#include <upc_relaxed.h>

#define ARRAY_SIZE 500000
#define MAX_VAL 1000
#define MIN_VAL -1000

// Shared array accessible by all threads
shared [] double src[ARRAY_SIZE];
shared [ARRAY_SIZE] double dst[ARRAY_SIZE*THREADS];
unsigned int * seed;

void swap(double *a, double *b) {
    double temp = *a;
    *a = *b;
    *b = temp;
}

int partition(double *arr, int low, int high) {
    double pivot = arr[high];
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

void quicksort(double *arr, int low, int high) {
    if (low < high) {
        int p = partition(arr, low, high);
        quicksort(arr, low, p - 1);
        quicksort(arr, p + 1, high);
    }
}

int main() {
    seed = (unsigned int *)malloc(sizeof(unsigned int));
    *seed = time(NULL)*1234 + MYTHREAD;
    srand(*seed);
    upc_tick_t time_start, time_end;
    double time_elapsed;
    int chunk_size = ARRAY_SIZE / THREADS;
    int src_start = MYTHREAD * chunk_size;
    //int end = (MYTHREAD == THREADS - 1) ? ARRAY_SIZE : start + chunk_size;
    int src_end = (MYTHREAD + 1) * chunk_size;
    int dst_start = src_start + (MYTHREAD * ARRAY_SIZE);
    int dst_end = src_end + (MYTHREAD * ARRAY_SIZE);
    /* printf("Thread %d: chunk_size=%d, start=%d, end=%d\n", MYTHREAD, chunk_size, src_start, src_end);
    printf("Thread %d: global start=%d, end=%d\n", MYTHREAD, dst_start, dst_end); */

    if(MYTHREAD==0) 
        for(int i = 0; i < ARRAY_SIZE; i++)
            src[i] = MIN_VAL + ((double)rand_r(seed) / RAND_MAX) * (MAX_VAL - MIN_VAL);
    upc_barrier;
    time_start = upc_ticks_now();
    upc_all_scatter(dst, src, sizeof(double)*ARRAY_SIZE, UPC_IN_MYSYNC | UPC_OUT_MYSYNC);
    time_end = upc_ticks_now();
    upc_barrier;
    
    quicksort((double *)&dst[dst_start], 0, chunk_size - 1);
    
    upc_all_gather(dst, &dst[dst_start],  sizeof(double) * chunk_size, UPC_IN_MYSYNC | UPC_OUT_MYSYNC);
    
    upc_barrier;

    // Verify if the local subarray is sorted
    int is_sorted = 1;
    for (int i = dst_start; i < dst_end - 1; i++) {
        if (((double *)dst)[i] > ((double *)dst)[i + 1]) {
            is_sorted = 0;
            printf("Thread %d: %f < %f !!!\n", MYTHREAD, ((double *)dst)[i], ((double *)dst)[i + 1]);
            break;
        }
    }
    if (is_sorted) {
        printf("Subarray of thread %d is sorted correctly\n", MYTHREAD);
    } else {
        printf("Subarray of thread %d is sorted incorrectly\n", MYTHREAD);
    }
    upc_barrier;
    time_elapsed = upc_ticks_to_ns(time_end - time_start);
    if (MYTHREAD == 0) 
        printf("Elapsed time for main calculation in milliseconds:\n");
    printf("Thread %d - %f milliseconds\n", MYTHREAD, time_elapsed/1000000.0);

    return 0;
}

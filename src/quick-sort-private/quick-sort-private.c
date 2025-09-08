#include <upc.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <upc_tick.h>
#include <upc_collective.h>
#include <upc_relaxed.h>

#define N 400000
#define MAX_VAL 1000
#define MIN_VAL -1000

shared [] double dane[N];
shared [N/THREADS] double dane_lokalne[N];
unsigned int * ziarno;

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
    ziarno = (unsigned int *)malloc(sizeof(unsigned int));
    *ziarno = time(NULL)*1234 + MYTHREAD;
    srand(*ziarno);
    upc_tick_t czas_start, czas_stop;
    double czas;
    int start = MYTHREAD * (N / THREADS);
    int stop = start + N/THREADS;

    if(MYTHREAD==0) 
        for(int i = 0; i < N; i++)
            dane[i] = MIN_VAL + ((double)rand_r(ziarno) / RAND_MAX) * (MAX_VAL - MIN_VAL);
    upc_barrier;

    czas_start = upc_ticks_now();
    upc_all_scatter(dane_lokalne, dane, sizeof(double) * (N/THREADS), UPC_IN_MYSYNC | UPC_OUT_MYSYNC);
    quicksort((double *)&dane_lokalne[start], 0, N/THREADS-1);
    upc_all_gather(dane, dane_lokalne,  sizeof(double) * (N/THREADS), UPC_IN_MYSYNC | UPC_OUT_MYSYNC);
    czas_stop = upc_ticks_now();


    upc_barrier;

    int is_sorted = 1;
    for (int i = start; i < stop - 1; i++) {
        //if (((double *)dane_lokalne)[i] > ((double *)dane_lokalne)[i + 1]) {
        if( dane[i] > dane[i + 1]) {
            is_sorted = 0;
            printf("Thread %d: el. %d = %f < %f = el. %d !!!\n", MYTHREAD, i, dane[i], dane[i + 1], i + 1);
            break;
        }
    }
    if (is_sorted) {
        printf("Subarray of thread %d is sorted correctly\n", MYTHREAD);
    } else {
        printf("Subarray of thread %d is sorted incorrectly\n", MYTHREAD);
    }
    upc_barrier;

    
    czas = upc_ticks_to_ns(czas_stop - czas_start)/1000000.0;
    if (MYTHREAD == 0) 
        printf("Elapsed time for main calculation in milliseconds:\n");
        fflush(stdout);
    printf("Thread %d - %f milliseconds\n", MYTHREAD, czas);

    return 0;
}

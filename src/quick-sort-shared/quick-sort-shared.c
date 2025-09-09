#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <upc_tick.h>
#include <upc_collective.h>
#include <upc_relaxed.h>

#define N 400000
#define MAX 1000
#define MIN -1000

shared [] double dane[N];
shared [N] double dane_lokalne[THREADS][N];
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
            dane[i] = MIN + ((double)rand_r(ziarno) / RAND_MAX) * (MAX - MIN);
    upc_barrier;

    czas_start = upc_ticks_now();
    upc_all_broadcast(dane_lokalne, dane, sizeof(double)*N, UPC_IN_MYSYNC | UPC_OUT_MYSYNC);
    quicksort((double *)&dane_lokalne[MYTHREAD][start], 0, N/THREADS-1);
    upc_all_gather_all(dane_lokalne, &dane_lokalne[MYTHREAD][start],  sizeof(double) * (N/THREADS), UPC_IN_MYSYNC | UPC_OUT_MYSYNC);
    czas_stop = upc_ticks_now();
    
    upc_barrier;
    czas = upc_ticks_to_ns(czas_stop - czas_start)/1000000.0;
    if (MYTHREAD == 0) 
        printf("Czas wykonania w milisekundach:\n");
        fflush(stdout);
    printf("Wątek %d - %f milisekund\n", MYTHREAD, czas);

    return 0;
}

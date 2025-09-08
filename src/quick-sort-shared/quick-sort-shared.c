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
shared [N] double dane_lokalne[THREADS][N];
unsigned int * ziarno;

void swap(double *a, double *b) {
    double temp = *a;
    *a = *b;
    *b = temp;
}

#define RAND_VAL (MIN_VAL + ((double)rand_r(ziarno) / RAND_MAX) * (MAX_VAL - MIN_VAL))
#define PRINT_FMT "%f 

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
            dane[i] = RAND_VAL;
    upc_barrier;

    czas_start = upc_ticks_now();
    upc_all_broadcast(dane_lokalne, dane, sizeof(double)*N, UPC_IN_MYSYNC | UPC_OUT_MYSYNC);
    quicksort((double *)&dane_lokalne[MYTHREAD][start], 0, N/THREADS-1);
    upc_all_gather_all(dane_lokalne, &dane_lokalne[MYTHREAD][start],  sizeof(double) * (N/THREADS), UPC_IN_MYSYNC | UPC_OUT_MYSYNC);
    czas_stop = upc_ticks_now();

    
    upc_barrier;

    if(MYTHREAD==0){
        int is_sorted=1;
        for(int i = 0; i< THREADS; i++) {
            for(int j = 0; j < N-1; j++) {
                if(dane_lokalne[i][j] > dane_lokalne[i][j+1] && (j+1)%(N/THREADS) != 0) {

                    is_sorted=0;
                    printf("Watek %d: el. %d = %f < %f = el. %d !!!\n", i, j, dane_lokalne[i][j], dane_lokalne[i][j+1], j+1);
                    //break;
                }
            }
        }
    }

    czas = upc_ticks_to_ns(czas_stop - czas_start);
    if (MYTHREAD == 0) 
        printf("Elapsed time for main calculation in milliseconds:\n");
        fflush(stdout);
    printf("Thread %d - %f milliseconds\n", MYTHREAD, czas/1000000.0);

    return 0;
}

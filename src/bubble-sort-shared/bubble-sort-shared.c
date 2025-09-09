#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <upc_tick.h>
#include <upc_strict.h>
#include <upc_collective.h>
#include <upc_relaxed.h>

#define MIN -1000
#define MAX 1000
#define N 4000

shared [] int dane[N];
shared [N/THREADS] int dane_lokalne[N];
unsigned int * ziarno;

void bubble_sort() {
    int temp, phase;
    int start = MYTHREAD * (N / THREADS);
    int stop = start + N/THREADS;
    printf("Thread %d sorting from %d to %d\n", MYTHREAD, start, stop);
    
    for (phase = 0; phase < N; phase++)
        for (int i = start; i < stop - 1; i++)
            if (dane_lokalne[i] > dane_lokalne[i + 1]) {
                temp = dane_lokalne[i];
                dane_lokalne[i] = dane_lokalne[i + 1];
                dane_lokalne[i + 1] = temp;
            }
}

int main() {
    // Inicjalizacja ziarna losowego
    ziarno = (unsigned int *)malloc(sizeof(unsigned int));
    *ziarno = time(NULL)*1234 + MYTHREAD;
    srand(*ziarno);

    upc_tick_t czas_start, czas_stop;
    double czas;

    if(MYTHREAD==0)
        for(int i = 0; i < N; i++) 
            dane[i] = MIN + (rand_r(ziarno) % (MAX - MIN + 1));
    upc_barrier;

    czas_start = upc_ticks_now();
    upc_all_scatter(dane_lokalne, dane, sizeof(int)*(N/THREADS), UPC_IN_NOSYNC | UPC_OUT_NOSYNC);
    bubble_sort();
    upc_all_gather(dane, dane_lokalne, sizeof(int)*(N/THREADS), UPC_IN_NOSYNC | UPC_OUT_NOSYNC);
    czas_stop = upc_ticks_now();

    czas = upc_ticks_to_ns(czas_stop - czas_start)/1000000.0;
    if (MYTHREAD == 0) {
        printf("\nCzas wykonania w milisekundach:\n");
        fflush(stdout);
    }
    printf("Wątek %d - %f milisekund\n", MYTHREAD, czas);
    return 0;
}

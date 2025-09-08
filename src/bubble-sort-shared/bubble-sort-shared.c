#include <upc.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <upc_tick.h>
#include <upc_strict.h>
#include <upc_collective.h>


#define LOWER_BOUND -1000
#define UPPER_BOUND 1000

#define N 4000

shared [] int dane[N];
shared [N/THREADS] int dane_lokalne[N];


unsigned int * ziarno;

void verify_sorted() {
    int start = MYTHREAD * (N / THREADS);
    int end = start + N/THREADS;
    for (int i = start; i < end-1; i++) {
        if (dane_lokalne[i] > dane_lokalne[i + 1]) {
            return;
        }
    }
}

void bubble_sort() {
    // Zmienne pomocnicze
    int temp, phase;
    int start = MYTHREAD * (N / THREADS);
    int end = start + N/THREADS;
    printf("Thread %d sorting from %d to %d\n", MYTHREAD, start, end);
    
    for (phase = 0; phase < N; phase++) {
        for (int i = start; i < end - 1; i++) {
            if (dane_lokalne[i] > dane_lokalne[i + 1]) {
                    // Swap
                temp = dane_lokalne[i];
                dane_lokalne[i] = dane_lokalne[i + 1];
                dane_lokalne[i + 1] = temp;
            }
        }
    }
        verify_sorted();
    

}

int main() {
    // Inicjalizacja ziarna losowego
    ziarno = (unsigned int *)malloc(sizeof(unsigned int));
    *ziarno = time(NULL)*1234 + MYTHREAD;
    srand(*ziarno);

    // Zmienne pomocnicze do pomiaru czasu
    upc_tick_t czas_start, czas_stop;
    double czas;
    
    // Inicjalizacja tabeli początkowej
    if(MYTHREAD==0) {
        for(int i = 0; i < N; i++) 
            dane[i] = LOWER_BOUND + (rand_r(ziarno) % (UPPER_BOUND - LOWER_BOUND + 1));

    }      
    upc_barrier;

    
    czas_start = upc_ticks_now();
    upc_all_scatter(dane_lokalne, dane, sizeof(int)*(N/THREADS), UPC_IN_NOSYNC | UPC_OUT_NOSYNC);
    upc_barrier;
    bubble_sort();
    upc_all_gather(dane, dane_lokalne, sizeof(int)*(N/THREADS), UPC_IN_NOSYNC | UPC_OUT_NOSYNC);
    czas_stop = upc_ticks_now();

    
    czas = upc_ticks_to_ns(czas_stop - czas_start)/1000000.0;
    if (MYTHREAD == 0) {
        printf("\nElapsed time for main calculation in milliseconds:\n");
        fflush(stdout);
    }
    upc_barrier;
    printf("Thread %d - %f milliseconds\n", MYTHREAD, czas);
    return 0;
}

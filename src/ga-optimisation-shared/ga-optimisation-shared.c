#include <upc.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <time.h>
#include <upc_tick.h>
#include <upc_relaxed.h>
#include <upc_collective.h>
#define N 20
#define SRC_THREAD 0

#define IL_OSOBNIKOW 200    // Rozmiar populacji
#define IL_GENOW 10  // Ilość zmiennych osobnika
#define MAX_EPOK 10000    // Ilość pokoleń
#define SZANSA_MUTACJI 0.05   // Współczynnik mutacji
#define SZANSA_KRZYZOWANIA 0.8  // Współczynnik krzyżowania
#define LOWER_BOUND -5.0    // Dolna granica poszukiwań
#define UPPER_BOUND 5.0     // Górna granica poszukiwań

shared double najlepsza_ocena = DBL_MAX;
shared double najlepsze_geny[IL_GENOW];
typedef struct {
    double geny[IL_GENOW];
    double ocena;
} Osobnik;
shared [] Osobnik nowa_populacja[IL_OSOBNIKOW];
shared [IL_OSOBNIKOW] Osobnik populacja[IL_OSOBNIKOW*THREADS];


unsigned int * seed;

// Optymalizowana funkcja
double evaluate(const double *x) {
    double sum = 0.0;
    for (int i = 0; i < IL_GENOW; ++i)
        sum += x[i]*x[i];
    return sum+100.0;
}

// Selekcja turniejowa
int tournament_select(shared Osobnik *pop, int pop_per_thread) {
    int a = rand_r(seed) % pop_per_thread;
    int b = rand_r(seed) % pop_per_thread;
    return (pop[MYTHREAD*IL_OSOBNIKOW + MYTHREAD * pop_per_thread + a].ocena <
            pop[MYTHREAD*IL_OSOBNIKOW + MYTHREAD * pop_per_thread + b].ocena) ? a : b;
}

// Krzyżowanie
void crossover(const Osobnik *p1, const Osobnik *p2, Osobnik *child) {
    for (int i = 0; i < IL_GENOW; i++) 
        child->geny[i] = (((double)rand_r(seed) / RAND_MAX) < 0.5) ? p1->geny[i] : p2->geny[i];
}

// Mutacja
void mutate(Osobnik *ind) {
    for (int i = 0; i < IL_GENOW; i++) 
        if (((double)rand_r(seed) / RAND_MAX) < SZANSA_MUTACJI) {
            double mutation_amount = 0.1 * ((double)rand_r(seed)/RAND_MAX - 0.5);
            ind->geny[i] += mutation_amount;
            if (ind->geny[i] > UPPER_BOUND) ind->geny[i] = UPPER_BOUND;
            if (ind->geny[i] < LOWER_BOUND) ind->geny[i] = LOWER_BOUND;
        }
}

int main() {
    // Inicjalizacja ziarna losowego (prywatnie dla każdego wątku)
    ziarno = (unsigned int *)malloc(sizeof(unsigned int));
    *ziarno = time(NULL)*1234 + MYTHREAD;
    srand(*ziarno);

    // Zmienne pomocnicze
    upc_tick_t czas_start, czas_stop;
    double czas;
    int os_na_watek = IL_OSOBNIKOW / THREADS;
    int start = (MYTHREAD*IL_OSOBNIKOW) + (MYTHREAD*os_na_watek);
    int stop = start + os_na_watek;
    upc_barrier;

    // Inicjalizacja pierwszej populacji
    if (MYTHREAD == 0) {
         printf("Initial data:\n");
        printf("Thread %d: Local  size = %llu\n", MYTHREAD, upc_localsizeof(nowa_populacja));
        printf("Thread %d: Block  size = %llu\n", MYTHREAD, upc_blocksizeof(nowa_populacja));
        printf("Thread %d: Elem size = %llu\n", MYTHREAD, upc_elemsizeof(nowa_populacja));
        for (int i = 0; i < IL_OSOBNIKOW; i++) {
            for(int j = 0; j < IL_GENOW; j++)
                nowa_populacja[i].geny[j] = LOWER_BOUND + ((double)rand_r(ziarno) / RAND_MAX) * (UPPER_BOUND - LOWER_BOUND);
            nowa_populacja[i].ocena = evaluate(nowa_populacja[i].geny);
            printf("%f ", nowa_populacja[i].geny[0]);
        }
        printf("\n\n");
    }
    
    upc_lock_t *zamek = upc_all_lock_alloc();
    upc_barrier;
    
    upc_all_broadcast(populacja, nowa_populacja, sizeof(Osobnik)*IL_OSOBNIKOW, UPC_IN_NOSYNC | UPC_OUT_NOSYNC);
    czas_start = upc_ticks_now();
    for (int gen = 0; gen < MAX_EPOK; gen++) {
        Osobnik elita = populacja[0];
        for (int i = 1; i < IL_OSOBNIKOW; i++) {
            if (populacja[i].ocena < elita.ocena)
            elita = populacja[i];
        }
        for (int i = start; i < stop; i++) {
            int p1_idx = tournament_select(populacja, os_na_watek) + (IL_OSOBNIKOW*MYTHREAD);
            int p2_idx = tournament_select(populacja, os_na_watek) + (IL_OSOBNIKOW*MYTHREAD);
            Osobnik nowy;
            if (((double)rand() / RAND_MAX) < SZANSA_KRZYZOWANIA)
                crossover(&populacja[p1_idx], &populacja[p2_idx], &nowy);
            else
                nowy = populacja[p1_idx];
            mutate(&nowy);
            nowy.ocena = evaluate(nowy.geny);
            fflush(stdout);
            nowa_populacja[i-start] = nowy;

            upc_lock(zamek);
            if (nowy.ocena < najlepsza_ocena) {
                najlepsza_ocena = nowy.ocena;
                for (int d = 0; d < IL_GENOW; d++)
                    najlepsze_geny[d] = nowy.geny[d];
            }
            upc_unlock(zamek);
        }
        if (MYTHREAD == 0)
            nowa_populacja[0] = elita;
        upc_barrier;
        upc_all_broadcast(populacja, nowa_populacja, sizeof(Osobnik)*IL_OSOBNIKOW, UPC_IN_NOSYNC | UPC_OUT_NOSYNC);
    }
    czas_stop = upc_ticks_now();

    upc_barrier;
    czas = upc_ticks_to_ns(czas_stop - czas_start);

    if (MYTHREAD == 0) {
        
        printf("Najlepsza ocena: %f\n", najlepsza_ocena);
        printf("Najlepsze geny: ");
        for (int i = 0; i < IL_GENOW; i++) {
            printf("%f ", najlepsze_geny[i]);
        }
        printf("\nCzasy przetwarzania:\n");
        fflush(stdout);
    }
    printf("Watek %d - %f milisekund\n", MYTHREAD, czas/1000000.0);
}
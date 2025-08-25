#include <upc.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <time.h>
#include <upc_tick.h>

#define POP_SIZE 100    // Rozmiar populacji
#define DIM 10  // Ilość zmiennych osobnika
#define GENERATIONS 20000    // Ilość pokoleń
#define MUTATION_RATE 0.1   // Współczynnik mutacji
#define CROSSOVER_RATE 0.7  // Współczynnik krzyżowania
#define LOWER_BOUND -5.0    // Dolna granica poszukiwań
#define UPPER_BOUND 5.0     // Górna granica poszukiwań

// Struktura przedstawiająca osobnika
typedef struct {
    double genes[DIM];
    double fitness;
} Individual;

// Wszystke zmienne przechowywane w pamięci współdzielonej
shared double best_global_fitness = DBL_MAX;
shared double best_global_solution[DIM];
shared unsigned int thread_seeds[THREADS];
//shared Individual population[POP_SIZE];
//shared Individual new_population[POP_SIZE];
shared [POP_SIZE] Individual population[POP_SIZE*THREADS];
shared [] Individual new_population[POP_SIZE];

void print_table(shared Individual *pop) {
    if (MYTHREAD == 0) {
        printf("Population:\n");
        for (int i = 0; i < 3; i++) {
            printf("Individual %d: ", i);
            for (int j = 0; j < DIM; j++) {
                printf("%f ", pop[i].genes[j]);
            }
            printf("Fitness: %f\n", pop[i].fitness);
        }
    }
}

double rand_double(double min, double max) {
    return min + ((double)rand_r(&thread_seeds[MYTHREAD]) / RAND_MAX) * (max - min);
}

// Funkcja testująca do optymalizacji
// f(x) = sum((x_i^4 - 16*x_i^2 + 5*x_i)) + 200.0
// Minimum globalne jest niezerowe, nietrywialne do znalezienia
double evaluate(const double *x) {
    double sum = 0.0;
    for (int i = 0; i < DIM; ++i) {
        double xi = x[i];
        sum += (xi * xi * xi * xi) - 16.0 * (xi * xi) + 5.0 * xi;
    }
    return sum + 200.0;
}

// Inicjalizacja osobnika
void init_individual(Individual *ind) {
    for (int i = 0; i < DIM; i++) {
        ind->genes[i] = rand_double(LOWER_BOUND, UPPER_BOUND);
        }
    ind->fitness = evaluate(ind->genes);
    ind->fitness = evaluate(ind->genes);
}

// Selekcja turniejowa
int tournament_select(shared Individual *pop, int pop_per_thread) {
    int a = rand_r(&thread_seeds[MYTHREAD]) % pop_per_thread;
    int b = rand_r(&thread_seeds[MYTHREAD]) % pop_per_thread;
    return (pop[MYTHREAD*POP_SIZE + MYTHREAD * pop_per_thread + a].fitness <
            pop[MYTHREAD*POP_SIZE + MYTHREAD * pop_per_thread + b].fitness) ? a : b;
    //return (pop[MYTHREAD * pop_per_thread + a].fitness <
    //        pop[MYTHREAD * pop_per_thread + b].fitness) ? a : b;
}

// Krzyżowanie
void crossover(const Individual *p1, const Individual *p2, Individual *child) {
    for (int i = 0; i < DIM; i++) {
        child->genes[i] = (((double)rand_r(&thread_seeds[MYTHREAD]) / RAND_MAX) < 0.5) ? p1->genes[i] : p2->genes[i];
    }
}

// Mutacja
void mutate(Individual *ind) {
    for (int i = 0; i < DIM; i++) {
        // Use thread-local seed
        if (((double)rand_r(&thread_seeds[MYTHREAD]) / RAND_MAX) < MUTATION_RATE) {
            ind->genes[i] += rand_double(-0.5, 0.5);
            if (ind->genes[i] < LOWER_BOUND) ind->genes[i] = LOWER_BOUND;
            if (ind->genes[i] > UPPER_BOUND) ind->genes[i] = UPPER_BOUND;
        }
    }
}

// Główna pętla optymalizacji genetycznej
void genetic_algorithm() {
    upc_tick_t time_start, time_end;
    double time_elapsed;
    int pop_per_thread = POP_SIZE / THREADS;
    //int start = MYTHREAD * pop_per_thread;
    int start = (MYTHREAD*POP_SIZE) + (MYTHREAD*pop_per_thread);
    int end = start + pop_per_thread;


    // Inicjalizacja całej populacji
    for (int i = start; i < end; i++)
        init_individual(&population[i]);
    // Synchronizacja wątków, by uniknąć operacji na niezainicjalizowanej populacji
    upc_barrier;

    //TABELA NEW_POP ROZMIARU N - WSZYSTKIE WĄTKI SPISUJĄ DO NIEJ WYNIK ALFORYTMU,
    //   AFFINITY NIEWAŻNE BO MAŁO OPERACJI
    //TABELA POP JEST ROZMIARU N*THREADS - KAŻDY WĄTEK OPERUJE NA LICZNIKU POWIEKSZONYM O N*MYTHREAD
    
    // Alokacja zamka do kontroli dostępu do zmiennych współdzielonych
    upc_lock_t *lock = upc_all_lock_alloc();
    time_start = upc_ticks_now();
    for (int gen = 0; gen < GENERATIONS; gen++) {
        for (int i = start; i < end; i++) {
            int p1_idx = tournament_select(population, pop_per_thread) + start;
            int p2_idx = tournament_select(population, pop_per_thread) + start;
            Individual child;
            if (((double)rand() / RAND_MAX) < CROSSOVER_RATE)
                crossover(&population[p1_idx], &population[p2_idx], &child);
            else
                child = population[p1_idx];
            mutate(&child);
            child.fitness = evaluate(child.genes);
            new_population[i] = child;

            upc_lock(lock);
            if (child.fitness < best_global_fitness) {
                best_global_fitness = child.fitness;
                for (int d = 0; d < DIM; d++)
                    best_global_solution[d] = child.genes[d];
            }
            upc_unlock(lock);
        }
        //Synchronizacja wątków, by uniknąć konfliktów przy zapisie do współdzielonej populacji
        /*upc_barrier;
        if(MYTHREAD==0){
            printf("pop BEFORE copy\n");
            print_table(population);
            printf("new pop BEFORE copy\n");
            print_table(new_population);
        }
        upc_barrier;*/
        for (int i = start; i < end; i++)
            population[i] = new_population[i];
            /*
        upc_barrier;
        if(MYTHREAD==0){
            printf("pop AFTER copy\n");
            print_table(population);
            printf("new pop AFTER copy\n");
            print_table(new_population);
        }*/
        upc_barrier;
        if (MYTHREAD == 0) {
            printf("Generation %d, Best Fitness: %f\n", gen, best_global_fitness);
            printf("-------------------------------------------------------------------------------");
        }
    }
    //upc_free(lock);
    time_end = upc_ticks_now();
    upc_barrier;
    time_elapsed = upc_ticks_to_ns(time_end - time_start);

    if (MYTHREAD == 0) {
        
        printf("Final Best Fitness: %f\n", best_global_fitness);
        printf("Best Solution: ");
        for (int i = 0; i < DIM; i++) {
            printf("%f ", best_global_solution[i]);
        }
        printf("\nElapsed time for main calculation in milliseconds:\n");
    }
    // Synchronizacja wątków w celu wymuszenia kolejności wypisywania czasu
    upc_barrier;
    printf("Thread %d - %f milliseconds\n", MYTHREAD, time_elapsed/1000000.0);
}

int main() {
    // Inicjalizacja ziarna losowania dla każdego wątku
    thread_seeds[MYTHREAD] = time(NULL)*1234 + MYTHREAD;
    upc_barrier;
    genetic_algorithm();
    return 0;
}

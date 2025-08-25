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

#define POP_SIZE 200    // Rozmiar populacji
#define DIM 10  // Ilość zmiennych osobnika
#define GENERATIONS 1000    // Ilość pokoleń
#define MUTATION_RATE 0.05   // Współczynnik mutacji
#define CROSSOVER_RATE 0.8  // Współczynnik krzyżowania
#define LOWER_BOUND -5.0    // Dolna granica poszukiwań
#define UPPER_BOUND 5.0     // Górna granica poszukiwań

shared double best_global_fitness = DBL_MAX;
shared double best_global_solution[DIM];

typedef struct {
    double genes[DIM];
    double fitness;
} Individual;

shared [] Individual new_population[POP_SIZE];
shared [POP_SIZE] Individual population[POP_SIZE*THREADS];
unsigned int * seed;


double evaluate(const double *x) {
    double sum = 0.0;
    for (int i = 0; i < DIM; ++i)
        sum += x[i]*x[i];
    //printf("Thread %d evaluated fitness: %f with first value: %f\n", MYTHREAD, sum, x[0]);
    return sum+100.0;
}

int tournament_select(shared Individual *pop, int pop_per_thread) {
    int a = rand_r(seed) % pop_per_thread;
    int b = rand_r(seed) % pop_per_thread;
    return (pop[MYTHREAD*POP_SIZE + MYTHREAD * pop_per_thread + a].fitness <
            pop[MYTHREAD*POP_SIZE + MYTHREAD * pop_per_thread + b].fitness) ? a : b;
    //return (pop[MYTHREAD * pop_per_thread + a].fitness <
    //        pop[MYTHREAD * pop_per_thread + b].fitness) ? a : b;
}

// Krzyżowanie
void crossover(const Individual *p1, const Individual *p2, Individual *child) {
    for (int i = 0; i < DIM; i++) {
        child->genes[i] = (((double)rand_r(seed) / RAND_MAX) < 0.5) ? p1->genes[i] : p2->genes[i];
    }
}

// Mutacja
void mutate(Individual *ind) {
    //printf("Thread %d: input value to mutate: %f\n", MYTHREAD, ind->genes[0]);
    fflush(stdout);
    for (int i = 0; i < DIM; i++) {
        //ind->genes[i] += 0.1;
        // Use thread-local seed
        if (((double)rand_r(seed) / RAND_MAX) < MUTATION_RATE) {
            //double mutation_amount = (double)rand_r(seed)/RAND_MAX - 0.5; // Random value in range [-0.5, 0.5]
            double mutation_amount = 0.1 * ((double)rand_r(seed)/RAND_MAX - 0.5);
            ind->genes[i] += mutation_amount;
            //printf("Thread %d: mutated gene %d by %f\n", MYTHREAD, i, mutation_amount);
            if (ind->genes[i] > UPPER_BOUND) ind->genes[i] = UPPER_BOUND;
            if (ind->genes[i] < LOWER_BOUND) ind->genes[i] = LOWER_BOUND;
        }
/*         else
            printf("Thread %d skipped mutation of gene %d\n", MYTHREAD, i); */
    }
    //printf("Thread %d: output value to mutate: %f\n", MYTHREAD, ind->genes[0]);
    fflush(stdout);
}

double rand_double(double min, double max) {
    return min + ((double)rand_r(seed) / RAND_MAX) * (max - min);
}

/*void init_individuals(shared Individual *pop) {

        for (int i = MYTHREAD; i < POP_SIZE; i+=THREADS)  {
            for(int j = 0; j < DIM; j++) {
                pop[i].genes[j] = rand_double(LOWER_BOUND, UPPER_BOUND);  
            }
            printf("thread %d initied individual %d with first value: %f\n", MYTHREAD, i, pop[i].genes[0]);
            pop[i].fitness = evaluate(pop[i].genes);
        }
    

    return;
}*/


int main() {
    seed = (unsigned int *)malloc(sizeof(unsigned int));
    *seed = time(NULL)*1234 + MYTHREAD;
    srand(*seed);
    
    upc_tick_t time_start, time_end;
    double time_elapsed;
    int pop_per_thread = POP_SIZE / THREADS;
    int start = (MYTHREAD*POP_SIZE) + (MYTHREAD*pop_per_thread);
    int end = start + pop_per_thread;
    upc_barrier;
    if (MYTHREAD == 0) {
         printf("Initial data:\n");
        printf("Thread %d: Local  size = %llu\n", MYTHREAD, upc_localsizeof(new_population));
        printf("Thread %d: Block  size = %llu\n", MYTHREAD, upc_blocksizeof(new_population));
        printf("Thread %d: Elem size = %llu\n", MYTHREAD, upc_elemsizeof(new_population));
        for (int i = 0; i < POP_SIZE; i++) {
            for(int j = 0; j < DIM; j++)
                new_population[i].genes[j] = LOWER_BOUND + ((double)rand_r(seed) / RAND_MAX) * (UPPER_BOUND - LOWER_BOUND);
            new_population[i].fitness = evaluate(new_population[i].genes);
            printf("%f ", new_population[i].genes[0]);
        }
        printf("\n\n");
    }
    upc_barrier;

    upc_all_broadcast(population, new_population, sizeof(Individual)*POP_SIZE, UPC_IN_ALLSYNC | UPC_OUT_ALLSYNC);


    //algorytm genetyczny:
    
    // Alokacja zamka do kontroli dostępu do zmiennych współdzielonych
    upc_lock_t *lock = upc_all_lock_alloc();
    time_start = upc_ticks_now();
    for (int gen = 0; gen < GENERATIONS; gen++) {
        // Elitism: find best individual
        Individual elite = population[0];
        for (int i = 1; i < POP_SIZE; i++) {
            if (population[i].fitness < elite.fitness)
            elite = population[i];
        }
        for (int i = start; i < end; i++) {
            int p1_idx = tournament_select(population, pop_per_thread) + (POP_SIZE*MYTHREAD);
            int p2_idx = tournament_select(population, pop_per_thread) + (POP_SIZE*MYTHREAD);
            Individual child;
            if (((double)rand() / RAND_MAX) < CROSSOVER_RATE)
                crossover(&population[p1_idx], &population[p2_idx], &child);
            else
                child = population[p1_idx];
            mutate(&child);
            child.fitness = evaluate(child.genes);
            fflush(stdout);
            new_population[i-start] = child;

            upc_lock(lock);
            if (child.fitness < best_global_fitness) {
                best_global_fitness = child.fitness;
                for (int d = 0; d < DIM; d++)
                    best_global_solution[d] = child.genes[d];
            }
            upc_unlock(lock);
        }
        if (MYTHREAD == 0)
            new_population[0] = elite;
        //Synchronizacja wątków, by uniknąć konfliktów przy zapisie do współdzielonej populacji
        upc_barrier;
        //upc_all_broadcast(population, new_population, sizeof(Individual)*POP_SIZE, UPC_IN_NOSYNC | UPC_OUT_NOSYNC);
        upc_all_broadcast(population, new_population, sizeof(Individual)*POP_SIZE, UPC_IN_ALLSYNC | UPC_OUT_ALLSYNC);
        upc_barrier;
        fflush(stdout);
        /* if (MYTHREAD == 0) {
            printf("Generation %d, Best Fitness: %f\n", gen, best_global_fitness);
            fflush(stdout);
        } */
    }
    time_end = upc_ticks_now();
    upc_barrier;
    time_elapsed = upc_ticks_to_ns(time_end - time_start);

    if (MYTHREAD == 0) {
        
        printf("Final Best Fitness: %f\n", best_global_fitness);
        printf("Best Solution: ");
        for (int i = 0; i < DIM; i++) {
            printf("%f ", best_global_solution[i]);
        }
/*         printf("\nFull end population:");
        for (int i = 0; i < POP_SIZE*THREADS; i++) {
            printf("\nIndividual %d: ", i);
            for(int j = 0; j < DIM; j++)
                printf("%f ", population[i].genes[j]);
        } */
        printf("\nElapsed time for main calculation in milliseconds:\n");
    }
    // Synchronizacja wątków w celu wymuszenia kolejności wypisywania czasu
    upc_barrier;
    printf("Thread %d - %f milliseconds\n", MYTHREAD, time_elapsed/1000000.0);
}
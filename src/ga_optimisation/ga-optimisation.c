#include <upc.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <time.h>

#define POP_SIZE 100       // Total population
#define DIM 10             // Problem dimensionality
#define GENERATIONS 100    // Number of generations
#define MUTATION_RATE 0.1  // Mutation rate
#define CROSSOVER_RATE 0.7 // Crossover rate
#define LOWER_BOUND -5.0
#define UPPER_BOUND 5.0

shared double best_global_fitness = DBL_MAX;
shared double best_global_solution[DIM];

// Structure for individual
typedef struct {
    double genes[DIM];
    double fitness;
} Individual;

// Shared population divided across threads
shared Individual population[POP_SIZE];
shared Individual new_population[POP_SIZE];

// Utility functions
double rand_double(double min, double max) {
    return min + ((double)rand() / RAND_MAX) * (max - min);
}

// Objective function: Sphere
double evaluate(const double *x) {
    double sum = 0.0;
    for (int i = 0; i < DIM; i++)
        sum += x[i] * x[i];
    return sum;
}

// Initialize individual
void init_individual(Individual *ind) {
    srand(time(NULL) + MYTHREAD * 1000);
    for (int i = 0; i < DIM; i++) {
        ind->genes[i] = rand_double(LOWER_BOUND, UPPER_BOUND);
        //printf("Initiated genes: %d\n", ind->genes[i]); }
    ind->fitness = evaluate(ind->genes);
    printf("Initiated fitness: %d\n", ind->fitness);
}

// Tournament selection
int tournament_select(shared Individual *pop, int pop_per_thread) {
    int a = rand() % pop_per_thread;
    int b = rand() % pop_per_thread;
    return (pop[MYTHREAD * pop_per_thread + a].fitness <
            pop[MYTHREAD * pop_per_thread + b].fitness) ? a : b;
}

// Crossover
void crossover(const Individual *p1, const Individual *p2, Individual *child) {
    for (int i = 0; i < DIM; i++) {
        if (((double)rand() / RAND_MAX) < 0.5)
            child->genes[i] = p1->genes[i];
        else
            child->genes[i] = p2->genes[i];
    }
}

// Mutation
void mutate(Individual *ind) {
    for (int i = 0; i < DIM; i++) {
        if (((double)rand() / RAND_MAX) < MUTATION_RATE) {
            ind->genes[i] += rand_double(-0.5, 0.5);
            if (ind->genes[i] < LOWER_BOUND) ind->genes[i] = LOWER_BOUND;
            if (ind->genes[i] > UPPER_BOUND) ind->genes[i] = UPPER_BOUND;
        }
    }
}

// GA loop
void genetic_algorithm() {
    int pop_per_thread = POP_SIZE / THREADS;
    int start = MYTHREAD * pop_per_thread;
    int end = start + pop_per_thread;

    // Initialize population
    for (int i = start; i < end; i++) {
        init_individual(&population[i]);
    }

    upc_barrier;

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

            // Update best individual locally
            if (child.fitness < best_global_fitness) {
                upc_lock_t *lock = upc_all_lock_alloc();
                upc_lock(lock);
                if (child.fitness < best_global_fitness) {
                    best_global_fitness = child.fitness;
                    for (int d = 0; d < DIM; d++)
                        best_global_solution[d] = child.genes[d];
                }
                upc_unlock(lock);
            }
        }

        upc_barrier;

        // Copy new population
        for (int i = start; i < end; i++) {
            population[i] = new_population[i];
        }

        upc_barrier;

        if (MYTHREAD == 0 && gen % 1 == 0) {
            printf("Generation %d, Best Fitness: %f\n", gen, best_global_fitness);
        }
    }

    if (MYTHREAD == 0) {
        printf("Final Best Fitness: %f\n", best_global_fitness);
        printf("Best Solution: ");
        for (int i = 0; i < DIM; i++) {
            printf("%f ", best_global_solution[i]);
        }
        printf("\n");
    }
}

int main() {
    srand(time(NULL) * 1000);
    genetic_algorithm();
    return 0;
}

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
unsigned int seed;

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
    //unsigned int seed = time(NULL)*1234 + MYTHREAD; // Thread-specific seed
    //printf("Seed for thread %d: %u\n", MYTHREAD, seed);
    return min + ((double)rand_r(&seed) / RAND_MAX) * (max - min);
}

// Objective function: Sphere
double evaluate2(const double *x) {
    double sum = 0.0;
    for (int i = 0; i < DIM; i++)
        sum += x[i] * x[i];
    return sum;
}

double evaluate3(const double *x) {
    double product = 1.0;
    for (int i = 0; i < DIM; i++)
        product *= sin(x[i]);
    return product*1000+100.0; // Example: Sphere function with a constant offset
    // offset added to ensure non-zero global minimum
}


// Custom benchmark function (non-convex, non-zero minimum):
// f(x) = sum((x_i^4 - 16*x_i^2 + 5*x_i)) + 200.0
// Global minimum is non-zero and non-trivial to find
double evaluate(const double *x) {
    double sum = 0.0;
    for (int i = 0; i < DIM; ++i) {
        double xi = x[i];
        sum += (xi * xi * xi * xi) - 16.0 * (xi * xi) + 5.0 * xi;
    }
    return sum + 200.0;
}

/*
Each term has multiple local minima and maxima.

It combines polynomial terms → nonlinear, non-convex, smooth.

The added constant +200.0 ensures global minimum is not zero.

Can be evaluated within a bounded domain, e.g. 

This is a great choice for evolutionary algorithms like genetic algorithms, because:

It presents a rugged landscape.

No trigonometric functions—uses only arithmetic and powers.

Global minimum is not at the origin and not zero, making convergence tracking more interesting.
*/

// Initialize individual
void init_individual(Individual *ind) {
    for (int i = 0; i < DIM; i++) {
        ind->genes[i] = rand_double(LOWER_BOUND, UPPER_BOUND);
        //printf("Initiated genes: %f\n", ind->genes[i]); 
        }
    ind->fitness = evaluate(ind->genes);
    printf("Initiated fitness: %f\n", ind->fitness);
}

// Tournament selection
int tournament_select(shared Individual *pop, int pop_per_thread) {
    //unsigned int seed = time(NULL) + MYTHREAD; // Thread-specific seed
    int a = rand_r(&seed) % pop_per_thread;
    int b = rand_r(&seed) % pop_per_thread;
    // 'a' and 'b' are already within valid range due to modulo operation in rand_r
    return (pop[MYTHREAD * pop_per_thread + a].fitness <
            pop[MYTHREAD * pop_per_thread + b].fitness) ? a : b;
}

// Crossover
void crossover(const Individual *p1, const Individual *p2, Individual *child) {
    for (int i = 0; i < DIM; i++) {
        //unsigned int seed = time(NULL) + MYTHREAD; // Thread-specific seed
        child->genes[i] = (((double)rand_r(&seed) / RAND_MAX) < 0.5) ? p1->genes[i] : p2->genes[i];
    }
}

// Mutation
void mutate(Individual *ind) {
    for (int i = 0; i < DIM; i++) {
        //unsigned int seed = time(NULL) + MYTHREAD; // Thread-specific seed
        if (((double)rand_r(&seed) / RAND_MAX) < MUTATION_RATE) {
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
    for (int i = start; i < end; i++)
        init_individual(&population[i]);
    

    upc_barrier;
    
    upc_lock_t *lock = upc_all_lock_alloc(); // Allocate lock outside the loop
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
             // Acquire the lock before checking
            //upc_lock(lock);
            //printf("locked by thread %d\n", MYTHREAD);
            
            if (child.fitness < best_global_fitness) {
                
                
                best_global_fitness = child.fitness;
                for (int d = 0; d < DIM; d++)
                    best_global_solution[d] = child.genes[d];
                
            }
            // Release the lock after updating
            //upc_unlock(lock);
            //printf("unlocked by thread %d\n", MYTHREAD);
             
            }
        
        upc_barrier;
    
    
    
    // Copy new population
        for (int i = start; i < end; i++)
            population[i] = new_population[i];
        upc_free(lock); // Free the lock after all generations
        upc_barrier;
        if (MYTHREAD == 0) {
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
    seed = time(NULL)*1234 + MYTHREAD;
    genetic_algorithm();
    return 0;
}

#include <upc.h>
#include <stdio.h>
#include <stdlib.h>
#include <upc_tick.h>
#include <upc_strict.h>
//KOLEJNY PRZYKŁAD ZMIANY Z RELAXED NA STRICT? JEŚLI TAK, TO TRZEBA BĘDZIE DOBRZE SIE O TYM ROZPISAĆ

#define N 50000       // Number of items
#define W 500      // Capacity of knapsack

// Shared arrays for dynamic programming
shared int dp[2][W + 1];  // We toggle between 2 rows

shared [1] int weights[N];
shared [1] int values[N];



int main() {
    int i, w;
    upc_tick_t time_start, time_end;
    double time_elapsed;
    if (MYTHREAD == 0) {
        for (int i = 0; i < N; i++) {
            weights[i] = rand() % 11;
            values[i] = rand() % 11;
            //printf("Item %d: Weight = %d, Value = %d\n", i, weights[i], values[i]);
        }
    }

    if (MYTHREAD == 0) {
        printf("Parallel 0/1 Knapsack using Berkeley UPC\n");
        printf("Items: %d, Capacity: %d, Threads: %d\n\n", N, W, THREADS);
    }

    upc_barrier;

    // Start timing
    time_start = upc_ticks_now();

    for (i = 0; i < N; i++) {
        int curr = i % 2;
        int prev = (i + 1) % 2;

        //ALGORYTM NIE STWARZA SYTUACJI, GDZIE 2 WĄTKI PRZETWARZAJĄ TEN SAM RZĄD TABELI DP

        // Divide the work: each thread processes a chunk of weights
        for (w = MYTHREAD; w <= W; w += THREADS) {
            if (weights[i] <= w) {
                int include = values[i] + dp[prev][w - weights[i]];
                int exclude = dp[prev][w];
                dp[curr][w] = (include > exclude) ? include : exclude;
            } else {
                dp[curr][w] = dp[prev][w];
            }
        }

        // Ensure all threads are done before next item
        upc_barrier;
    }

    // End timing
    time_end = upc_ticks_now();
    time_elapsed = upc_ticks_to_ns(time_end - time_start);

    // Final result is in dp[(N-1)%2][W]
    if (MYTHREAD == 0) {
        int result = dp[(N-1)%2][W];
        printf("Maximum value in knapsack = %d\n", result);
        printf("Elapsed time for main calculation in milliseconds:\n");
    }
    printf("Thread %d - %f milliseconds\n", MYTHREAD, time_elapsed/1000000.0);

    return 0;
}

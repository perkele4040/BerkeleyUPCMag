#include <upc.h>
#include <stdio.h>
#include <stdlib.h>
#include <upc_tick.h>

#define N 200       // Number of items
#define W 500      // Capacity of knapsack

// Shared arrays for dynamic programming
shared int dp[2][W + 1];  // We toggle between 2 rows

shared int weights[N];
shared int values[N];



int main() {
    int i, w;
    upc_tick_t start, end;
    double elapsed;
    if (MYTHREAD == 0) {
        for (int i = 0; i < N; i++) {
            weights[i] = rand() % 15 + 1;
            values[i] = rand() % 16;
            printf("Item %d: Weight = %d, Value = %d\n", i, weights[i], values[i]);
        }
    }

    if (MYTHREAD == 0) {
        printf("Parallel 0/1 Knapsack using Berkeley UPC\n");
        printf("Items: %d, Capacity: %d, Threads: %d\n\n", N, W, THREADS);
    }

    upc_barrier;

    // Start timing
    start = upc_ticks_now();

    for (i = 0; i < N; i++) {
        int curr = i % 2;
        int prev = (i + 1) % 2;

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
    end = upc_ticks_now();
    elapsed = upc_ticks_to_ns(end - start);

    // Final result is in dp[(N-1)%2][W]
    if (MYTHREAD == 0) {
        int result = dp[(N-1)%2][W];
        printf("Maximum value in knapsack = %d\n", result);
        printf("Elapsed time for main calculation: %f nanoseconds\n", elapsed);
    }

    return 0;
}

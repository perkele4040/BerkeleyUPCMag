#include <upc.h>
#include <stdio.h>
#include <stdlib.h>

#define N 5       // Number of items
#define W 10      // Capacity of knapsack

// Shared arrays for dynamic programming
shared int dp[2][W + 1];  // We toggle between 2 rows

// Local arrays (each thread will own a part of these)
int weights[N] = {2, 3, 4, 5, 9};
int values[N] = {3, 4, 5, 8, 10};

int main() {
    int i, w;
    int tid = MYTHREAD;
    int nthreads = THREADS;

    if (MYTHREAD == 0) {
        printf("Parallel 0/1 Knapsack using Berkeley UPC\n");
        printf("Items: %d, Capacity: %d, Threads: %d\n\n", N, W, THREADS);
    }

    upc_barrier;

    for (i = 0; i < N; i++) {
        int curr = i % 2;
        int prev = (i + 1) % 2;

        // Divide the work: each thread processes a chunk of weights
        for (w = tid; w <= W; w += nthreads) {
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

    // Final result is in dp[(N-1)%2][W]
    if (MYTHREAD == 0) {
        int result = dp[(N-1)%2][W];
        printf("Maximum value in knapsack = %d\n", result);
    }

    return 0;
}

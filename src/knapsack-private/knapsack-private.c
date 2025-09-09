#include <stdio.h>
#include <stdlib.h>
#include <upc_tick.h>
#include <upc_strict.h>

#define N 50000
#define W 500

shared int plecak[2][W + 1];
shared [1] int wagi[N];
shared [1] int wartosci[N];

int main() {
    int i, w;
    if (MYTHREAD == 0) {
        for (int i = 0; i < N; i++) {
            wagi[i] = rand() % 11;
            wartosci[i] = rand() % 11;
        }
    }
    upc_barrier;

    upc_tick_t czas_start = upc_ticks_now();
    for (i = 0; i < N; i++) {
        int c = i % 2;
        int p = (i + 1) % 2;
        for (w = MYTHREAD; w <= W; w += THREADS) {
            if (wagi[i] <= w) {
                int include = wartosci[i] + plecak[p][w - wagi[i]];
                int exclude = plecak[p][w];
                plecak[c][w] = (include > exclude) ? include : exclude;
            } else {
                plecak[c][w] = plecak[p][w];
            }
        }
        upc_barrier;
    }
    upc_tick_t czas_stop = upc_ticks_now();
    double czas = upc_ticks_to_ns(czas_stop - czas_start)/1000000.0;

    if (MYTHREAD == 0) {
        int result = plecak[(N-1)%2][W];
        printf("Maksymalna wartosc plecaka = %d\n", result);
        printf("Czas wykonania w milisekundach:\n");
    }
    printf("Watek %d - %f milisekund\n", MYTHREAD, czas);

    return 0;
}

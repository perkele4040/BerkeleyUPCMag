#include<stdio.h>
#include<upc.h>
#include<math.h>
#include<upc_tick.h>
#ifndef M_PI
#define M_PI (3.14159265358979323846)
#endif
#define __UPC_TICK__ 1

int main()
{
    
    upc_lock_t *time_lock = upc_all_lock_alloc();
    upc_lock_t *outcome_lock = upc_all_lock_alloc();
    int max = 10e+7*2*4;
    int n = THREADS;
    double positive = 0.0;
    double negative = 0.0;
    double local_outcome = 0.0;
    shared double *global_outcome = upc_all_alloc(1, sizeof(double));  
    shared long unsigned int *global_time = upc_all_alloc(1, sizeof(long unsigned int));
    long unsigned int time_initial = 0;
    upc_memput(global_time, &time_initial, sizeof(unsigned long int));
    if(MYTHREAD == 0) {    
        printf("Poczatkowo %lf, czas w milis %lu\n", 4*(*global_outcome), upc_ticks_to_ns(*global_time)/1000000);
    }   
    //start of timer
    upc_tick_t local_time = upc_ticks_now();
    for(int i = MYTHREAD; i < max; i += n)
    {
        int j = 1 + 4*i;
        positive += 1.0 / j;
        negative += 1.0 / (j + 2.0);     
    }
    local_outcome = positive-negative;
    local_time = upc_ticks_now() - local_time;
    //end of timer
    printf("watek %d: wynik lokalny %lf, czas lokalny: %lu\n",MYTHREAD, 4*local_outcome, upc_ticks_to_ns(local_time)/1000000);  
    upc_lock(outcome_lock);
    local_outcome += *global_outcome;
    upc_memput(global_outcome, &local_outcome, sizeof(double));
    //printf("watek %d wynik globalny %lf wynik lokalny %lf\n",MYTHREAD, *wynik_globalny, wynik_lokalny);
    upc_unlock(outcome_lock);
    upc_lock(time_lock);
    local_time += *global_time;
    upc_memput(global_time, &local_time, sizeof(long unsigned int));
    //printf("watek %d czas globalny %lu czas lokalny %lu\n",MYTHREAD, *czas_globalny, czas_lokalny);
    upc_unlock(time_lock);
    upc_barrier;
    if(MYTHREAD == 0) { 
        printf("finalnie %lf, prawdziwe PI %lf, czas milis %lu\n", 4*(*global_outcome), M_PI, upc_ticks_to_ns(*global_time)/(1000000*n));
    }

    upc_global_exit(0);
}

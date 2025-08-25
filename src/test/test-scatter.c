#include <upc.h>
#include <stdio.h>
#include <upc_collective.h>
#define NELEMS N/THREADS
#define N 50
shared [] int array[N];
shared [NELEMS] int local_array[N];

//obrazek zrobic na forall relaxed vs strict, szukac roznic w czasie przetwarzania
//wzorzec scatter -> gather vs broadcast
//gdzie użyć ALLSYNC vs NOSYNC? może jakaś inna funkcja do agregacji



int main(int argc, char const *argv[])
{
    
    //array = (shared[] int *) upc_alloc(N*sizeof(int));
    //local_array = (shared [NELEMS] int *) upc_alloc(N*sizeof(int));
    //local_array = (shared [NELEMS] int *) upc_alloc(NELEMS*sizeof(int));
    if (MYTHREAD == 0) {
        printf("Initial array:\n");
        for (int i = 0; i < N; i++) {
            array[i] = i;
        }
        for (int i = 0; i < N; i++)
            printf("%d ", array[i]);
        printf("\n\n");
        printf("Some information about global array:\n\n");
    }
    upc_barrier;

    printf("Thread %d: Local  size = %llu\n", MYTHREAD, upc_localsizeof(*array));
    printf("Thread %d: Block  size = %llu\n", MYTHREAD, upc_blocksizeof(*array));
    printf("Thread %d: Elem size = %llu\n", MYTHREAD, upc_elemsizeof(*array));
    upc_barrier;
    printf("Thread %d: Address of array elem 5 : %p\n", MYTHREAD, (void*)&array[5]);
    printf("Thread %d: Value of array elem 5 :  %d\n", MYTHREAD, array[5]);
    //printf("Address of local  array elem 0 : %p\n", (void*)&array[0]);
    upc_barrier;
    size_t threadof = upc_threadof(array);
    printf("Thread %d: affinity of array = %zu\n", MYTHREAD, threadof);
    upc_barrier;
    printf("Thread %d: Address of local array before scatter elem 5 : %p\n", MYTHREAD, (void*)&local_array[5]);
    printf("Thread %d: Value of local array before scatter elem 5 :  %d\n", MYTHREAD, local_array[5*(MYTHREAD+1)]);
    
    upc_barrier;
    printf("thread %d do scatter\n", MYTHREAD);
    upc_all_scatter(local_array, array, NELEMS*sizeof(int), UPC_IN_NOSYNC | UPC_OUT_NOSYNC);

    if(MYTHREAD==0){
        printf("\nScatter complete\n");
        printf("\n\nSome information about local array:\n\n");
    }
    upc_barrier;
    printf("Thread %d: Local  size = %llu\n", MYTHREAD, upc_localsizeof(*local_array));
    printf("Thread %d: Block  size = %llu\n", MYTHREAD, upc_blocksizeof(*local_array));
    printf("Thread %d: Elem size = %llu\n", MYTHREAD, upc_elemsizeof(*local_array));
    upc_barrier;
    printf("Thread %d: Address of local array elem 5 : %p\n", MYTHREAD, (void*)&local_array[5]);
    printf("Thread %d: Value of local array elem 5 :  %d\n", MYTHREAD, local_array[5*(MYTHREAD+1)]);
    //printf("Address of local  array elem 0 : %p\n", (void*)&array[0]);
    upc_barrier;
    threadof = upc_threadof(&local_array);
    printf("Thread %d: affinity of array = %zu\n", MYTHREAD, threadof);

    if(MYTHREAD==0){
        printf("\nInitial array: \n");
        for (int i = 0; i < N; i++) {
            printf("%d ", array[i]);
        }
        printf("\nLocal array on thread %d : \n", MYTHREAD);
        for (int i = NELEMS*MYTHREAD; i < NELEMS*(MYTHREAD+1); i++) {
            printf("%d ", local_array[i]);
        }
    }
    upc_barrier;
    if(MYTHREAD!=0){
        printf("\nLocal array on thread %d : \n", MYTHREAD);
        for (int i = NELEMS*MYTHREAD; i < NELEMS*(MYTHREAD+1); i++) {
            printf("%d ", local_array[i]);
        }
    }
    return 0;
}

    /*
    int i=0;
    // dynamically allocate shared memory with affinity to thread THREADS-1
    if(MYTHREAD == THREADS -1)
        A=(shared[] int *)upc_alloc(N*sizeof(int));
    upc_barrier;
    // initialize A
    upc_forall(i=0;i<NELEMS*THREADS;i++;A+i)
        A[i]=i;
    upc_barrier;
    int done = 0;
    do{
        upc_all_scatter(B,A,sizeof(int)*NELEMS,UPC_IN_ALLSYNC|UPC_OUT_ALLSYNC);
        printf("scattered %d elements, thread %d\n", NELEMS, MYTHREAD);
        upc_barrier;
        printf("print after %dth scatter: ", done/(NELEMS*THREADS));
        for (int i = 0; i < 2; i++)
            printf("Thread %d: B[%d]=%d\n", MYTHREAD, i, B[i]);
        done += NELEMS;
        upc_barrier;
    } while(done < N);
    if(MYTHREAD==0){
        for (i = 0; i < N; i++)
            printf(" %d ", A[i]);
    }

    // verify results
    for(i=0;i<NELEMS*THREADS;i++)
        if(B[i] != i)
        {
            printf("Error: thread=%d,B[%d]=%d,expect=%d\n",MYTHREAD,i,B[i],i);
            upc_global_exit(FAILURE); /*terminate all threads and force to exit
            program
            
        }
    return SUCCESS;
}*/

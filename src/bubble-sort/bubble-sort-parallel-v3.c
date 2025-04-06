//#include <upc_relaxed.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
//#include <upc.h>
#define SIZE 10
#define THREADS 1
#define MYTHREAD 0
#define MAX 100
#define MIN 0
//shared [THREADS] double arr[SIZE];

void swapIntLocal(int* xp, int* yp){
   int temp = *xp;
   *xp = *yp;
   *yp = temp;
}

int main() {
    /*
    1. losuj zawartosc tabeli
    2. podziel tabele na chunki, uzyj ceil
    3. na kazdym watku odpal zwykly bubble sort dla chunka
    4. polacz tabele jednym przejsciem
    */
   //int arr[SIZE];
   int* arr = (int*)malloc(SIZE * sizeof(int));
   srand(time(NULL));
   for(int i = 0; i < SIZE; i++)
      arr[i] = rand() % (MAX - MIN + 1) + MIN;
   //for(int i = 0; i < SIZE; i++)
   //   printf("%d, ", arr[i]);
   int elems_per_threads = ceil ((double)SIZE/THREADS);
   //printf("elems per thread = %d\n", elems_per_threads);
   bool swapped;
   for( int i = MYTHREAD*elems_per_threads; i < (MYTHREAD+1)*elems_per_threads-1; i++ ) {
      swapped = false;
      printf("i=%d\n", i);
      for( int j = MYTHREAD*elems_per_threads; j < (MYTHREAD+1)*elems_per_threads-i-1; j++ ) {
         printf("checking %d and %d\n", arr[j], arr[j+1]);
         if( arr[j] > arr[j+1] ) {
            swapIntLocal(&arr[j], &arr[j + 1]);
            swapped=true;
            printf("swapped\n");
         }
         for(int i = 0; i < SIZE; i++)
            printf("%d, ", arr[i]);
         printf("\n");
      }
   }
   for(int i = 0; i < SIZE; i++)
      printf("%d, ", arr[i]);
}
#include <upc_relaxed.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <upc.h>
#define SIZE 10
//#define THREADS 3
//#define MYTHREAD 0
#define MAX 100
#define MIN 0
shared [THREADS] int arr[SIZE];
shared int elems_per_threads;

void swapIntLocal(int* xp, int* yp){
   int temp = *xp;
   *xp = *yp;
   *yp = temp;
}
//wszystko na jednej dzielonej tabeli, minimalna ilość komunikacji międzyprocesorowej
//maksymalne użycie pamięci wspólnej
int main() {
    /*
    1. losuj zawartosc tabeli
    2. podziel tabele na chunki, uzyj ceil
    3. na kazdym watku odpal zwykly bubble sort dla chunka
    4. polacz tabele jednym przejsciem
    */
   //int arr[SIZE];
   //1. LOSOWANIE
   //tylko main
   //int* arr = (int*)malloc(SIZE * sizeof(int));
   //int* sorted = (int*)malloc(SIZE * sizeof(int));
   if ( MYTHREAD == 0 ) {
      srand(time(NULL));
      for(int i = 0; i < SIZE; i++)
         arr[i] = rand() % (MAX - MIN + 1) + MIN;
      elems_per_threads = ceil ((double)SIZE/THREADS);
      printf("array before sorting:\n");
      for(int i = 0; i < SIZE; i++)
         printf("%d, ", arr[i]);
      printf("elems per thread = %d\n\n", elems_per_threads);
   }

   upc_barrier;

   //ILOŚĆ ELEMENTÓW NA WĄTEK
   //shared variable?
   int temp=0;
   printf("thread %d sorting from %d to %d\n\n", MYTHREAD, MYTHREAD*elems_per_threads, (MYTHREAD+1)*elems_per_threads);
   //tyle inkrementacji i ile elementów ma przetworzyć wątek
   for( int i = 0; i < elems_per_threads; i++ ) {
      //tyle inkrementacji j ile elementów pozostało nieprzetworzonych
      //od początku do końca zakresu przydzielonego wątkowi
      
      for( int j = MYTHREAD*elems_per_threads; j < (MYTHREAD+1)*elems_per_threads-i-1; j++ ) {
         for(int i = 0; i < SIZE; i++)
            printf("%d, ", arr[i]);
         printf("\n");
         printf("comparing %d with %d\n", arr[j], arr[j+1]);
         if( arr[j] > arr[j+1] ) {
            printf("swapped\n");
            temp=arr[j];
            arr[j]=arr[j+1];
            arr[j+1]=temp;
            //swapIntLocal(&arr[j], &arr[j + 1]);
         }
      }
   }
   upc_barrier;
   if ( MYTHREAD == 0 ) {
      printf("\n\narray after sorting chunks but before merge\n");
      for(int i = 0; i < SIZE; i++)
         printf("%d, ", arr[i]);
   }

   /*
   //MERGE
   //i j mogą być elementami tabeli o długości równej ilości wątków
   int i = 0, j = elems_per_threads, z = 2*elems_per_threads, k = 0;
   // One pass through both arrays
   while (i < elems_per_threads && j < 2*elems_per_threads && z < 3*elems_per_threads && k < SIZE) {
      if (arr[i] <= arr[j]) {
          sorted[k++] = arr[i++];
      } else {
          sorted[k++] = arr[j++];
      }
  }

  // Copy any remaining elements from a[]
  while (i < elems_per_threads && k < SIZE) {
   sorted[k++] = arr[i++];
  }

  // Copy any remaining elements from b[]
  while (j < 2*elems_per_threads && k < SIZE) {
   sorted[k++] = arr[j++];
  }
  while (z < 3*elems_per_threads && k < SIZE) {
   sorted[k++] = arr[z++];
  }
  printf("\narrays were merged\n");
  for(int i = 0; i < SIZE; i++)
      printf("%d, ", sorted[i]);
   */
}
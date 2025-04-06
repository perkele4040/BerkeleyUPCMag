#include <upc_relaxed.h>
#include <stdio.h>
#include <upc.h>
//#include "../common-functions/common-functions.h"
#define SIZE 100
shared [THREADS] double arr[SIZE];
shared int swapped;



void parallelBubbleSort() {
    printf("Thread %d entering sort function\n", MYTHREAD);
    int i, j;
    for (i = 0; i < SIZE - 1; i++) {
        swapped = 0;
        upc_forall(j = 0; j < SIZE - i - 1; j++; j) {
            if (arr[j] > arr[j + 1]) {
                swapInt(&arr[j], &arr[j + 1]);
                swapped = 1;
            }
        }

        // If no two elements were swapped by inner loop,
        // then break
        if (swapped == 1)
            break;
    }
}

int main(){
    if(MYTHREAD==0) {
        printf("Using %d threads\n", THREADS); 
        const char* filenameInput = "bubble-sort-test-input.txt";
        const char* filenameOutput = "bubble-sort-test-output.txt";
        const int* size = SIZE;
        int* arr = loadArrayFromFile(filenameInput, &size);
        printf("Starting array: \n");
        for (int i = 0; i < SIZE; i++)
            printf("%d, ", arr[i]); }
    parallelBubbleSort();
    if(MYTHREAD==0) {
        printf("Outcome array: \n");
        for (int i = 0; i < SIZE; i++)
            printf("%d, ", arr[i]);
        printf("did bubble sort work?  = %d\n", validateSortingOutput(arr, &size));
        writeArrayToOutputFile(filenameOutput, &size, arr); }
    return 0;
}
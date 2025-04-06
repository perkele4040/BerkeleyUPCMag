#include <upc_relaxed.h>
#include <stdio.h>
#include <upc.h>
#include "../common-functions/common-functions.h"
#define SIZE 100
shared [THREADS] double arr[SIZE];



void bubbleSort(int arr[], int n){
    printf("Thread %d entering sort function\n", MYTHREAD);
    int i, j;
    bool swapped;
    upc_forall (i = 0; i < n - 1; i++) {
        swapped = false;
        for (j = 0; j < n - i - 1; j++) {
            if (arr[j] > arr[j + 1]) {
                swapInt(&arr[j], &arr[j + 1]);
                swapped = true;
            }
        }

        // If no two elements were swapped by inner loop,
        // then break
        if (swapped == false)
            break;
    }
}


int main(){
    if(MYTHREAD==0) {
        printf("Using %d threads\n", THREADS); }
    const char* filenameInput = "bubble-sort-test-input.txt";
    const char* filenameOutput = "bubble-sort-test-output.txt";
    int size;
    int* arr = loadArrayFromFile(filenameInput, &size);
    printf("Starting array: \n");
    printArray(arr, SIZE);
    //printf("size of array = %d\n", size);
    parallelBubbleSort(arr, SIZE);
    printf("Outcome array: \n");
    printArray(arr, SIZE);
    printf("did bubble sort work?  = %d\n", validateSortingOutput(arr, size));
    writeArrayToOutputFile(filenameOutput, &size, arr);
    return 0;
}
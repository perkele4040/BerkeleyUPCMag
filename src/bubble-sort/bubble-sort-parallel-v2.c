#include <upc_relaxed.h>
#include <stdio.h>
#include <upc.h>
#include "../common-functions/common-functions.h"
#define SIZE 100
shared [THREADS] double arr[SIZE];
shared bool swapped;



void parallelBubbleSort() {
    printf("Thread %d entering sort function\n", MYTHREAD);
    int i, j;
    for (i = 0; i < SIZE - 1; i++) {
        swapped = false;
        upc_forall(j = 0; j < SIZE - i - 1; j++; j) {
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

void printArray(int arr[], int size){
    int i;
    for (i = 0; i < size; i++)
        printf("%d, ", arr[i]);
}

int main(){
    if(MYTHREAD==0) {
        printf("Using %d threads\n", THREADS); 
        const char* filenameInput = "bubble-sort-test-input.txt";
        const char* filenameOutput = "bubble-sort-test-output.txt";
        int size = SIZE;
        int* arr = loadArrayFromFile(filenameInput, &size);
        printf("Starting array: \n");
        printArray(arr, SIZE); }
    parallelBubbleSort();
    if(MYTHREAD==0) {
        printf("Outcome array: \n");
        printArray(&arr, SIZE);
        printf("did bubble sort work?  = %d\n", validateSortingOutput(arr, size));
        writeArrayToOutputFile(filenameOutput, &size, arr); }
    return 0;
}
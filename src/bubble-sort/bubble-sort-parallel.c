#include <stdio.h>
#include <upc.h>
#include "../common-functions/common-functions.h"
#define SIZE 100



//Algorytm
void parallelBubbleSort(int arr[], int N) {
    int localSize = N / THREADS;
    int start = MYTHREAD * localSize;
    int end = start + localSize;
    
    for (int phase = 0; phase < N; phase++) {
        bool swapped = false;

        // Odd phase: swap adjacent elements in pairs
        if (phase % 2 == 0) {
            printf("Watek %d tu byl\n", MYTHREAD)
            for (int i = start; i < end - 1; i += 2) {
                if (arr[i] > arr[i + 1]) {
                    swapInt(&arr[i], &arr[i + 1]);
                    swapped = true;
                }
            }
        }
        // Even phase: swap elements in offset positions
        else {
            for (int i = start + 1; i < end - 1; i += 2) {
                if (arr[i] > arr[i + 1]) {
                    swapInt(&arr[i], &arr[i + 1]);
                    swapped = true;
                }
            }
        }

        // Synchronize all threads
        upc_barrier;

        // Stop if no swaps occurred
        if (!swapped) {
            break;
        }
    }
}

int main(){
 
    const char* filenameInput = "bubble-sort-test-input.txt";
    const char* filenameOutput = "bubble-sort-test-output.txt";
    int size;
    int* numbers = loadArrayFromFile(filenameInput, &size);
    printf("size of array = %d\n", size);
    parallelBubbleSort(numbers, SIZE);
    printArray(numbers, SIZE);
    printf("did bubble sort work?  = %d\n", validateSortingOutput(numbers, size));
    writeArrayToOutputFile(filenameOutput, &size, numbers);
    return 0;
}

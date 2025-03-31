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
    #ifdef _WIN32 
    printf("Hey Geek it seems that"
           "you are working on a Windows OS.\n"); 
  
    #elif __linux__ 
        printf("Hey Geek it seems that you"
            "are working on a Linux OS.\n"); 
    
    #elif __unix__ 
        printf("Hey Geek it seems that you"
            "are working on a unix OS.\n"); 
 
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

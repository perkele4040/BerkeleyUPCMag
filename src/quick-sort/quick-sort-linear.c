// C program to implement Quick Sort Algorithm
#include <stdio.h>
#include "../common-functions/common-functions.h"

int partition(int arr[], int low, int high) {

    // Initialize pivot to be the first element
    int p = arr[low];
    int i = low;
    int j = high;

    while (i < j) {

        // Find the first element greater than
        // the pivot (from starting)
        while (arr[i] <= p && i <= high - 1) {
            i++;
        }

        // Find the first element smaller than
        // the pivot (from last)
        while (arr[j] > p && j >= low + 1) {
            j--;
        }
        if (i < j) {
            swapInt(&arr[i], &arr[j]);
        }
    }
    swapInt(&arr[low], &arr[j]);
    return j;
}

void quickSort(int arr[], int low, int high) {
    if (low < high) {

        // call partition function to find Partition Index
        int pi = partition(arr, low, high);

        // Recursively call quickSort() for left and right
        // half based on Partition Index
        quickSort(arr, low, pi - 1);
        quickSort(arr, pi + 1, high);
    }
}

int main() {
    const char* filenameInput = "quick-sort-test-input.txt";
    const char* filenameOutput = "quick-sort-test-output.txt";
    int size;
    int* numbers = loadArrayFromFile(filenameInput, &size);
    //int arr[] = { 4, 2, 5, 3, 1 };
    //int n = sizeof(arr) / sizeof(arr[0]);
    //printArray(numbers, size);
    // calling quickSort() to sort the given array
    quickSort(numbers, 0, size - 1);
    printArray(numbers, size);
    writeArrayToOutputFile(filenameOutput, &size, numbers);
    //for (int i = 0; i < n; i++)
    //    printf("%d ", arr[i]);

    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

extern void printArray(int arr[], int size){
    int i;
    for (i = 0; i < size; i++)
        printf("%d, ", arr[i]);
}
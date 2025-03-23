#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

void printArray(int arr[], int size){
    int i;
    for (i = 0; i < size; i++)
        printf("%d, ", arr[i]);
}

void swapInt(int* xp, int* yp){
    int temp = *xp;
    *xp = *yp;
    *yp = temp;
}

bool validateSortingOutput(int arr[], int size) {
    for (int i = 1; i < size; i++){
        if(arr[i]<arr[i-1])
        return false;
    }
    return true;
}

void writeArrayToOutputFile(const char* filename, int* size, int arr[]) {
    FILE* file = fopen(filename, "w");
    if (!file) {
        perror("Error opening output file");
    }
    else {
        for(int i=0; i<*size-1; i++) {
            fprintf(file, "%d\n", arr[i]);
        }
        fprintf(file, "%d", arr[*size-1]);
        fclose(file);
    }
}

int* loadArrayFromFile(const char* filename, int* size) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Error opening input file");
        return NULL;
    }

    int capacity = 10;  // Initial capacity
    int* array = (int*)malloc(capacity * sizeof(int));
    if (!array) {
        perror("Memory allocation failed");
        fclose(file);
        return NULL;
    }

    int count = 0;
    int num;
    while (fscanf(file, "%d", &num) == 1) {
        if (count >= capacity) {
            capacity *= 2;
            int* temp = (int*)realloc(array, capacity * sizeof(int));
            if (!temp) {
                perror("Memory reallocation failed");
                free(array);
                fclose(file);
                return NULL;
            }
            array = temp;
        }
        array[count++] = num;
    }

    fclose(file);
    *size = count;
    return array;
}
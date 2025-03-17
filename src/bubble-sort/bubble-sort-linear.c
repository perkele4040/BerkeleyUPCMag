#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

void swap(int* xp, int* yp){
    int temp = *xp;
    *xp = *yp;
    *yp = temp;
}

//Algorytm
void bubbleSort(int arr[], int n){
    int i, j;
    bool swapped;
    for (i = 0; i < n - 1; i++) {
        swapped = false;
        for (j = 0; j < n - i - 1; j++) {
            if (arr[j] > arr[j + 1]) {
                swap(&arr[j], &arr[j + 1]);
                swapped = true;
            }
        }

        // If no two elements were swapped by inner loop,
        // then break
        if (swapped == false)
            break;
    }
}

bool validateSortingOutput(int arr[], int size) {
    for (int i = 1; i < size; i++){
        if(arr[i]<arr[i-1])
        return false;
    }
    return true;
}

// Function to print an array
void printArray(int arr[], int size){
    int i;
    for (i = 0; i < size; i++)
        printf("%d, ", arr[i]);
}

void writeArrayToOutputFile(const char* filename, int* size, int arr[]) {
    FILE* file = fopen(filename, "w");
    printf("size within write function = %d\n", *size);
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
    //return;
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

int main(){

    const char* filenameInput = "bubble-sort-test-input.txt";
    const char* filenameOutput = "bubble-sort-test-output.txt";
    int size;
    int* numbers = loadArrayFromFile(filenameInput, &size);
    printf("size of array = %d\n", size);
    bubbleSort(numbers, size);
    printArray(numbers, size);
    printf("did bubble sort work? = %d\n", validateSortingOutput(numbers, size));
    writeArrayToOutputFile(filenameOutput, &size, numbers);
    
    //printf("Sorted array: \n");
    //printArray(arr, n);
    return 0;
}

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>


//move to lib??
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

//Funkcja validateSortingOutput przyjmuje tabelę liczb całkowitych i jej rozmiar
//oraz sprawdza poprawność posortowania zawartości tabeli.
//Wynik walidacji zwracany jest typem BOOL.
bool validateSortingOutput(int arr[], int size) {
    for (int i = 1; i < size; i++){
        if(arr[i]<arr[i-1])
        return false;
    }
    return true;
}

//Funkcja printArray wypisuje w konsoli w jednej linii podaną tabelę liczb całkowitych.
void printArray(int arr[], int size){
    int i;
    for (i = 0; i < size; i++)
        printf("%d, ", arr[i]);
}

//Funkcja writeArrayToOutputFile przyjmuje wskaźnik do pliku .txt oraz tabelę liczb całkowitych i jej rozmiar.
//Funkcja zapisuje liczby do podanego pliku w nowych liniach oraz zwraca błąd w przypadku nieodnalezienia pliku.
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


//Funkcja loadArrayFromFile przyjmuje wskaźnik do pliku .txt zawierającego ciąg liczb całkowitych podanych w nowych liniach.
//Funkcja zwraca błąd w przypadku nieodnalezienia pliku lub problemów z alokacją pamięci.
//Liczby zwracane są w postaci tabeli INT.
//Alokacja pamięci na potrzeby tabeli przeprowadzana jest dynamiczne w zależności do rozmiaru wejścia.
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
    return 0;
}

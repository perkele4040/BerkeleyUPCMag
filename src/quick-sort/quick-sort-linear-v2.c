#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#define SIZE 10
#define THREADS 1
#define MAX 100
#define MIN 0



// Function that performs the Quick Sort
// for an array arr[] starting from the
// index start and ending at index end
void quicksort(int* arr, int start, int end)
{
    int pivot, index, temp;

    // Base Case
    if (end <= 1)
        return;

    // Pick pivot and swap with first
    // element Pivot is middle element
    pivot = arr[start + end / 2];
    temp=arr[start];
    arr[start]=arr[start + end / 2];
    arr[start + end / 2]=temp;
    //swap(arr, start, start + end / 2);

    // Partitioning Steps
    index = start;

    // Iterate over the range [start, end]
    for (int i = start + 1; i < start + end; i++) {

        // Swap if the element is less
        // than the pivot element
        if (arr[i] < pivot) {
            index++;
            //local swap
            temp=arr[i];
            arr[i]=arr[index];
            arr[index]=temp;
            //swap(arr, i, index);
        }
    }

    // Swap the pivot into place
    temp=arr[start];
    arr[start]=arr[index];
    arr[index]=temp;
    //swap(arr, start, index);

    // Recursive Call for sorting
    // of quick sort function
    quicksort(arr, start, index - start);
    quicksort(arr, index + 1, start + end - index - 1);
}

int main() {
    int elems_per_threads;
    int arr[SIZE];
    srand(time(NULL));
    for(int i = 0; i < SIZE; i++)
       arr[i] = rand() % (MAX - MIN + 1) + MIN;
    elems_per_threads = ceil ((double)SIZE/THREADS);
    printf("array before sorting:\n");
    for(int i = 0; i < SIZE; i++)
       printf("%d, ", arr[i]);
    printf("\nelems per thread = %d\n\n", elems_per_threads);
    quicksort(arr, 0, elems_per_threads);
    printf("array after sorting:\n");
    for(int i = 0; i < SIZE; i++)
       printf("%d, ", arr[i]);
}
 

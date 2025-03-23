//#include "C:\Users\kobyl\Desktop\studia\magister\GIT\BerkeleyUPCMag\src\common-functions\common-functions.h"
#include <stdio.h>

extern void printArray(int arr[], int size){
    int i;
    for (i = 0; i < size; i++)
        printf("%d, ", arr[i]);
}
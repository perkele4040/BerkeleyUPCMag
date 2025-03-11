#include<stdio.h>
#include<upc_strict.h>
#include <time.h>
#include <stdlib.h>
#include<stdbool.h>

int main()
{
    printf("Watek %d: Hello World!\n", MYTHREAD);
    upc_global_exit(0);
}

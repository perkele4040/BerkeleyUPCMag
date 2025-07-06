#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define MYTHREAD 0 // For testing purposes, set to 0
#define THREADS 4 // For testing purposes, set to 1
#define LOWER_BOUND -5.0
#define UPPER_BOUND 5.0
// Function to generate a random double between min and max
double rand_double(double min, double max) {
    //return min + (rand() / (double)RAND_MAX) * (max - min);
    return min + ((double)rand() / RAND_MAX) * (max - min);
}

void print_random_double(double value) {
    printf("Random double: %.2f\n", rand_double(LOWER_BOUND, UPPER_BOUND));
}

int main() {
    // Seed the random number generator
    //srand((unsigned int)time(NULL));
    //srand(time(NULL)*1000);

    // Define the range
    double min = -5.0;
    double max = 5.0;

    for(int i = 0; i < 10; i++) {
        print_random_double(rand_double(min, max));
    }
    return 0;
}
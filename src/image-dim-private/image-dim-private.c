#include <upc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <upc_tick.h>
#include <upc_strict.h>

#define GAMMA 2.0f
#define WIDTH 2000
#define HEIGHT 2980
#define TOTALSIZE (WIDTH * HEIGHT)
#define NELEMS (TOTALSIZE/(THREADS))

// Struktura przechowujaca dane piksela
typedef struct
{
    unsigned int r, g, b;
} Pixel;

// Wszystkie dane sa przechowywane w pamieci wspoldzielonej
shared [] Pixel * image_data;
shared int average_time;

Pixel applyGammaCorrection(Pixel p)
{
    Pixel corrected;
    // Zmiana gamma dla kazdego kanalu RGB
    // Poprawka gamma: corrected = 255 * (p / 255) ^ gamma
    corrected.r = (unsigned int)(255.0 * pow((double)p.r / 255.0, GAMMA));
    corrected.g = (unsigned int)(255.0 * pow((double)p.g / 255.0, GAMMA));
    corrected.b = (unsigned int)(255.0 * pow((double)p.b / 255.0, GAMMA));
    return corrected;
}

// Funkcja wczytujaca obraz z pliku PPM
//Funkcja zaklada, ze obraz nie posiada komentarzy
void load_ppm(const char *filename, Pixel **data)
{
    FILE *fp = fopen(filename, "r");
    if (!fp)
    {
        perror("File open failed");
        exit(EXIT_FAILURE);
    }
    char header[3];
    if (fscanf(fp, "%2s", header) != 1 || strcmp(header, "P3") != 0)
    {
        fprintf(stderr, "Unsupported format (only ASCII P3 allowed)\n");
        exit(EXIT_FAILURE);
    }
    int file_width, file_height, maxval;
    if (fscanf(fp, "%d %d", &file_width, &file_height) != 2)
    {
        fprintf(stderr, "Failed to read image dimensions\n");
        exit(EXIT_FAILURE);
    }
    if (fscanf(fp, "%d", &maxval) != 1)
    {
        fprintf(stderr, "Failed to read max color value\n");
        exit(EXIT_FAILURE);
    }
    
    *data = malloc(sizeof(Pixel) * file_width * file_height);
    for (size_t i = 0; i < (size_t)(file_width * file_height); i++)
    {
        int r, g, b;
        int ret=fscanf(fp, "%d %d %d", &r, &g, &b);
        if (ret != 3)
        {
            fprintf(stderr, "Failed to read pixel data\n");
            exit(EXIT_FAILURE);
        }
        (*data)[i].r = (unsigned int)r;
        (*data)[i].g = (unsigned int)g;
        (*data)[i].b = (unsigned int)b;
    }
    
    fclose(fp);
}

void save_ppm(const char *filename, shared [] Pixel *data)
{
    FILE *fp = fopen(filename, "w");
    if (!fp)
    {
        perror("Save file open failed");
        exit(EXIT_FAILURE);
    }
    fprintf(fp, "P3\n%d %d\n255\n", WIDTH, HEIGHT);
    for (size_t i = 0; i < TOTALSIZE; i++)
    {
        fprintf(fp, "%d %d %d\n", data[i].r, data[i].g, data[i].b);
    }
    fclose(fp);
}

int main()
{
    upc_tick_t time_start, time_end;
    double time_elapsed;
    average_time = 0;
    char inname[] = "mona-lisa-p3.ppm";
    char outname[] = "mona-lisa-corrected.ppm";

    if (MYTHREAD == 0) {
        Pixel *local_image;
        load_ppm(inname, &local_image);
        upc_memput(image_data, local_image, TOTALSIZE * sizeof(Pixel));
        free(local_image);

        printf("Image loaded from: %s\n", inname);
        printf("Image dimensions: %dx%d\n", WIDTH, HEIGHT);
        printf("Image size: %d pixels\n", TOTALSIZE);
    }

    upc_barrier;
    time_start = upc_ticks_now();
    upc_forall(int i = 0; i<TOTALSIZE; i++; i)
        image_data[i] = applyGammaCorrection(image_data[i]);
    time_end = upc_ticks_now();
    upc_barrier;

    time_elapsed = upc_ticks_to_ns(time_end - time_start);
    average_time += time_elapsed;
    upc_barrier;
    if(MYTHREAD==0) {
        printf("Elapsed time for main calculation in milliseconds:\n");
    }
    printf("Thread %d - %f milliseconds\n", MYTHREAD, time_elapsed/1000000.0);
    fflush(stdout);
    if(MYTHREAD==0)
        printf("Average time for main calculation in milliseconds: %f\n", average_time/THREADS);

    if (MYTHREAD == 0) {
        save_ppm(outname, image_data);
        printf("Inverted image saved as: %s\n", outname);
        
    }
    upc_free(image_data);

    return 0;
}
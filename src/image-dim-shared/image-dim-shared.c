#include <upc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <upc_tick.h>

#define GAMMA 2.0f

// Struktura przechowujaca dane piksela
typedef struct
{
    unsigned int r, g, b;
} Pixel;

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

// Wszystkie dane sa przechowywane w pamieci wspoldzielonej
shared Pixel *image_data;
shared const int width= 2000, height = 2980;
shared size_t img_size;
//Pomiar czasu przy pomocy biblioteki UPC
int local_start, local_end;

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
    
    *data = malloc(sizeof(Pixel) * img_size);
    for (size_t i = 0; i < img_size; i++)
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

void save_ppm(const char *filename, shared Pixel *data)
{
    FILE *fp = fopen(filename, "w");
    if (!fp)
    {
        perror("Save file open failed");
        exit(EXIT_FAILURE);
    }
    fprintf(fp, "P3\n%d %d\n255\n", width, height);
    for (size_t i = 0; i < img_size; i++)
    {
        fprintf(fp, "%d %d %d\n", data[i].r, data[i].g, data[i].b);
    }
    fclose(fp);
}

int main()
{
    
    upc_tick_t start, end;
    double elapsed;
    char inname[] = "mona-lisa-p3.ppm";
    char outname[] = "mona-lisa-corrected.ppm";

    img_size = width * height;
    Pixel *local_image = NULL;
    if (MYTHREAD == 0)
    {
        
        load_ppm(inname, &local_image);
    }
    image_data = upc_all_alloc(img_size, sizeof(Pixel));
    
    if (MYTHREAD == 0)
    {
        for (size_t i = 0; i < img_size; i++)
        {
            image_data[i] = local_image[i];
        }
        free(local_image);
        printf("Image loaded from: %s\n", inname);
        printf("Image dimensions: %dx%d\n", width, height);
        printf("Image size: %zu pixels\n", img_size);

    }

    upc_barrier;
    start = upc_ticks_now();
    
    // Parallel image inversion
    for (size_t i = MYTHREAD; i < img_size; i += THREADS) {
        image_data[i] = applyGammaCorrection(image_data[i]);
    }
    end = upc_ticks_now();
    upc_barrier;

    elapsed = upc_ticks_to_ns(end - start);
    if(MYTHREAD==0) {
        printf("Elapsed time for main calculation in milliseconds:\n");
    }
    printf("Thread %d - %f milliseconds\n", MYTHREAD, elapsed/1000000.0);
    // Master gathers data and saves
    if (MYTHREAD == 0) {
        // Save directly from shared image_data
        save_ppm(outname, image_data);
        printf("Inverted image saved as: %s\n", outname);
        upc_free(image_data);
    }

    return 0;
}
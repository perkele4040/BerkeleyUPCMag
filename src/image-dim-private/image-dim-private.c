#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <upc_tick.h>
#include <upc_strict.h>

#define WIDTH 2000
#define HEIGHT 2980
#define GAMMA 2.0f
#define ROZMIAR (WIDTH * HEIGHT)

typedef struct
{
    unsigned int r, g, b;
} Piksel;
shared [1] Piksel * obraz;

Piksel korekcja_gamma(Piksel p)
{
    Piksel corrected;
    corrected.r = (unsigned int)(255.0 * pow((double)p.r / 255.0, GAMMA));
    corrected.g = (unsigned int)(255.0 * pow((double)p.g / 255.0, GAMMA));
    corrected.b = (unsigned int)(255.0 * pow((double)p.b / 255.0, GAMMA));
    return corrected;
}

void otworz_obraz(const char *filename, Piksel **data)
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
        fprintf(stderr, "Obraz nie jest formatu PPM P3\n");
        exit(EXIT_FAILURE);
    }
    int file_width, file_height, maxval;
    if (fscanf(fp, "%d %d", &file_width, &file_height) != 2)
    {
        fprintf(stderr, "Brak wymiarow obrazu\n");
        exit(EXIT_FAILURE);
    }
    if (fscanf(fp, "%d", &maxval) != 1)
    {
        fprintf(stderr, "Brak maksymalnej wartosci koloru\n");
        exit(EXIT_FAILURE);
    }
    *data = malloc(sizeof(Piksel) * file_width * file_height);
    for (size_t i = 0; i < (size_t)(file_width * file_height); i++)
    {
        int r, g, b;
        int ret=fscanf(fp, "%d %d %d", &r, &g, &b);
        if (ret != 3)
        {
            fprintf(stderr, "Brak danych Pikseli\n");
            exit(EXIT_FAILURE);
        }
        (*data)[i].r = (unsigned int)r;
        (*data)[i].g = (unsigned int)g;
        (*data)[i].b = (unsigned int)b;
    }
    fclose(fp);
}

void zapisz_obraz(const char *filename, shared [] Piksel *data)
{
    FILE *fp = fopen(filename, "w");
    if (!fp)
    {
        perror("Save file open failed");
        exit(EXIT_FAILURE);
    }
    fprintf(fp, "P3\n%d %d\n255\n", WIDTH, HEIGHT);
    for (size_t i = 0; i < ROZMIAR; i++)
    {
        fprintf(fp, "%d %d %d\n", data[i].r, data[i].g, data[i].b);
    }
    fclose(fp);
}

int main()
{
    char plik_wej[] = "mona-lisa-p3.ppm";
    char plik_wyj[] = "mona-lisa-corrected.ppm";
    upc_tick_t czas_start, czas_stop;
    double czas;

    if (MYTHREAD == 0) {
        Piksel *obraz_lokalny;
        otworz_obraz(plik_wej, &obraz_lokalny);
        upc_memput(obraz, obraz_lokalny, ROZMIAR * sizeof(Piksel));
        free(obraz_lokalny);
    }
    upc_barrier;

    czas_start = upc_ticks_now();
    upc_forall(int i = 0; i<ROZMIAR; i++; i)
        obraz[i] = korekcja_gamma(obraz[i]);
    czas_stop = upc_ticks_now();
    
    czas = upc_ticks_to_ns(czas_stop - czas_start)/1000000.0;
    if(MYTHREAD==0) 
        printf("Czas wykonania w milisekundach:\n");
    printf("Watek %d - %f milisekund\n", MYTHREAD, czas);

    upc_barrier;
    if (MYTHREAD == 0) 
        zapisz_obraz(plik_wyj, obraz);
    return 0;
}
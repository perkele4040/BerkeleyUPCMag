#include <upc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_FILENAME 256

typedef struct {
    unsigned char r, g, b;
} Pixel;

shared Pixel *image_data; // Flattened shared image array
shared int width=2500, height=2500;
int local_start, local_end;

void load_ppm(const char *filename, Pixel **data) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        perror("File open failed");
        exit(EXIT_FAILURE);
    }

    char header[3];
    if (fscanf(fp, "%2s", header) != 1 || strcmp(header, "P3") != 0) {
        fprintf(stderr, "Unsupported format (only ASCII P3 allowed)\n");
        exit(EXIT_FAILURE);
    }

    // Skip comments
    int c;
    do { c = fgetc(fp); } while (c == '#');
    ungetc(c, fp);

    // Read dimensions
    //fscanf(fp, "%d %d", &width, &height);

    int maxval;
    fscanf(fp, "%d", &maxval);
    fgetc(fp); // consume newline

    size_t img_size = (width) * (height);
    *data = malloc(sizeof(Pixel) * img_size);
    for (size_t i = 0; i < img_size; i++) {
        int r, g, b;
        fscanf(fp, "%d %d %d", &r, &g, &b);
        (*data)[i].r = (unsigned char)r;
        (*data)[i].g = (unsigned char)g;
        (*data)[i].b = (unsigned char)b;
    }
    fclose(fp);
}

void save_ppm(const char *filename, Pixel *data) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        perror("Save file open failed");
        exit(EXIT_FAILURE);
    }
    fprintf(fp, "P3\n%d %d\n255\n", width, height);
    for (int i = 0; i < (width) * (height); i++) {
        fprintf(fp, "%d %d %d\n", data[i].r, data[i].g, data[i].b);
    }
    fclose(fp);
}

/*void create_image(const char *filename, int width, int height) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        perror("Save file open failed");
        exit(EXIT_FAILURE);
    }
    fprintf(fp, "P3\n%d %d\n255\n", width, height);
    fclose(fp);
}*/

int main(int argc, char **argv) {

    upc_barrier;
    char inname[] = "solid-color-image-p3.ppm";

    Pixel *local_image = NULL;
    if (MYTHREAD == 0) {
        load_ppm(inname, &local_image);
    }

    // Broadcast image size
    //upc_all_broadcast(&width, &width, sizeof(int), 0);
    //upc_all_broadcast(&height, &height, sizeof(int), 0);
    size_t img_size = (width) * (height);
    printf("thread %d here with im_size = %zu\n", MYTHREAD, img_size);
    // Allocate shared array
    image_data = upc_all_alloc(img_size, sizeof(Pixel));

    // Master thread copies loaded data to shared memory
    if (MYTHREAD == 0) {
        for (size_t i = 0; i < img_size; i++) {
            image_data[i] = local_image[i];
        }
        free(local_image);
    }

    upc_barrier;

    // Parallel image inversion
    for (size_t i = MYTHREAD; i < img_size; i += THREADS) {
        image_data[i].r = 255 - image_data[i].r;
        image_data[i].g = 255 - image_data[i].g;
        image_data[i].b = 255 - image_data[i].b;
        //printf("Thread %d inverted pixel %zu\n", MYTHREAD, i);
    }

    upc_barrier;
///*

    // Master gathers data and saves
    if (MYTHREAD == 0) {
        Pixel *output = malloc(sizeof(Pixel) * img_size);
        for (size_t i = 0; i < img_size; i++) {
            output[i] = image_data[i];
        }

        // Construct new filename
        char outname[] = "image-inverted.ppm";
        

        save_ppm(outname, output);
        printf("Inverted image saved as: %s\n", outname);
        free(output);
    }

    upc_barrier;
    //*/
    upc_free(image_data);

    return 0;
}
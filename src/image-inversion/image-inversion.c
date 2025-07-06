#include <upc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_FILENAME 256

typedef struct {
    unsigned char r, g, b;
} Pixel;

shared Pixel *image_data; // Flattened shared image array
int width, height;
int local_start, local_end;




void load_ppm(const char *filename, Pixel **data, int *width, int *height) {
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        perror("File open failed");
        exit(EXIT_FAILURE);
    }

    char header[3];
    if (fscanf(fp, "%2s", header) != 1 || strcmp(header, "P6") != 0) {
        fprintf(stderr, "Unsupported format (only binary P6 allowed)\n");
        exit(EXIT_FAILURE);
    }

    // Skip comments
    int c;
    do { c = fgetc(fp); } while (c == '#');
    ungetc(c, fp);

    // Read dimensions
    fscanf(fp, "%d %d", width, height);

    int maxval;
    fscanf(fp, "%d", &maxval);
    fgetc(fp); // consume newline

    size_t img_size = (*width) * (*height);
    *data = malloc(sizeof(Pixel) * img_size);
    fread(*data, sizeof(Pixel), img_size, fp);
    fclose(fp);
}

void save_ppm(const char *filename, Pixel *data, int width, int height) {
    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        perror("Save file open failed");
        exit(EXIT_FAILURE);
    }
    fprintf(fp, "P6\n%d %d\n255\n", width, height);
    fwrite(data, sizeof(Pixel), width * height, fp);
    fclose(fp);
}

void create_image(const char *filename, int width, int height) {
    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        perror("Save file open failed");
        exit(EXIT_FAILURE);
    }
    fprintf(fp, "P6\n%d %d\n255\n", width, height);
    //fwrite(data, sizeof(Pixel), width * height, fp);
    fclose(fp);
}

int main(int argc, char **argv) {


    char outname[] = "image-inverted.ppm";

    if (MYTHREAD == 0) {
        save_ppm(outname, width, height);
    }
    return 0;
}
    /*
    if (MYTHREAD == 0) {
        if (argc < 2) {
            fprintf(stderr, "Usage: %s <image.ppm>\n", argv[0]);
            upc_global_exit(1);
        }
    }

    upc_barrier;

    Pixel *local_image = NULL;
    if (MYTHREAD == 0) {
        load_ppm(argv[1], &local_image, &width, &height);
    }

    // Broadcast image size
    upc_all_broadcast(&width, &width, sizeof(int), 0);
    upc_all_broadcast(&height, &height, sizeof(int), 0);
    size_t img_size = width * height;

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
    }

    upc_barrier;

    // Master gathers data and saves
    if (MYTHREAD == 0) {
        Pixel *output = malloc(sizeof(Pixel) * img_size);
        for (size_t i = 0; i < img_size; i++) {
            output[i] = image_data[i];
        }

        // Construct new filename
        char outname[MAX_FILENAME];
        const char *dot = strrchr(argv[1], '.');
        if (dot) {
            snprintf(outname, MAX_FILENAME, "%.*s-inverted%s", (int)(dot - argv[1]), argv[1], dot);
        } else {
            snprintf(outname, MAX_FILENAME, "%s-inverted.ppm", argv[1]);
        }

        save_ppm(outname, output, width, height);
        printf("Inverted image saved as: %s\n", outname);
        free(output);
    }

    upc_barrier;

    upc_free(image_data);
    return 0;
}
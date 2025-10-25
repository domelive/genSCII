#ifndef IMAGE_H
#define IMAGE_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

typedef enum AllocationType {
    NO_ALLOCATION,
    SELF_ALLOCATED,
    STB_ALLOCATED
} AllocationType;

typedef struct Image {
    int width;
    int height;
    int channels;
    size_t size;
    uint8_t* data;
    AllocationType allocationType;
} Image;

static inline bool _strEndsWith(const char* str, const char* ends);

Image* Image_load(const char* filename);
Image* Image_create(int width, int height, int channels, bool zeroed);

void Image_save(const Image* img, const char* filename);
void Image_free(Image* img);

Image* Image_toGrayscale(const Image* original);

#endif // IMAGE_H

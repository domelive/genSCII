#include "Image.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../stb_image/stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../stb_image/stb_image_write.h"

static inline bool _strEndsWith(const char* str, const char* ends) {
    size_t str_len  = strlen(str);
    size_t ends_len = strlen(ends);

    char* pos = strstr(str, ends);

    return (pos != NULL) && (pos + ends_len == str + str_len);
}

Image* Image_load(const char *filename) {
    if (access(filename, F_OK) != 0) {
        fprintf(stderr, "File %s does not exist\n", filename);
        return NULL;
    }

    Image* out = malloc(sizeof(Image));
    if (!out) {
        fprintf(stderr, "Error allocating memory for image\n");
        return NULL;
    }

    out->data = stbi_load(filename, &out->width, &out->height, &out->channels, 0);
    if (!out->data) {
        fprintf(stderr, "Error loading image %s\n", filename);
        return NULL;
    }

    out->size = out->width * out->height * out->channels;
    out->allocationType = STB_ALLOCATED;

    return out; 
}

Image* Image_create(int width, int height, int channels, bool zeroed) {
    size_t size = width * height * channels;
    
    Image* out = malloc(sizeof(Image));
    if (!out) {
        fprintf(stderr, "Error allocating memory for image\n");
        return NULL;
    }

    out->data = (zeroed) ? calloc(size, 1) : malloc(size);
    if (!out->data) {
        fprintf(stderr, "Error allocating memory for image data\n");
        return NULL;
    }

    out->width = width;
    out->height = height;
    out->channels = channels;
    out->allocationType = SELF_ALLOCATED;

    return out;
}

// TODO: add more extensions
void Image_save(const Image *img, const char *filename) {
    if (_strEndsWith(filename, ".jpg") || _strEndsWith(filename, ".JPG") || _strEndsWith(filename, ".jpeg") || _strEndsWith(filename, ".JPEG")) {
        stbi_write_jpg(filename, img->width, img->height, img->channels, img->data, 100);
    } else if (_strEndsWith(filename, ".png") || _strEndsWith(filename, ".PNG")) {
        stbi_write_png(filename, img->width, img->height, img->channels, img->data, img->width * img->channels);
    } else {
        fprintf(stderr, "Image type not recognized. Unable to save image %s\n", filename);
        exit(EXIT_FAILURE);
    }
}

void Image_free(Image* img) {
    if (img->allocationType == NO_ALLOCATION || img->data == NULL) {
        fprintf(stderr, "Image not allocated\n");
        exit(EXIT_FAILURE);
    }

    if (img->allocationType == STB_ALLOCATED)
        stbi_image_free(img->data);
    else
        free(img->data);

    img->data = NULL;
    img->width = 0;
    img->height = 0;
    img->size = 0;
    img->allocationType = NO_ALLOCATION;
}

Image* Image_toGrayscale(const Image* original, GrayscaleMethod method) {
    // int channels = original->channels == 4 ? 2 : 1;
    Image* grayImg = Image_create(original->width, original->height, 1, false);
    if (!grayImg) {
        fprintf(stderr, "Error allocating image\n");
        return NULL;
    }

    for (int y = 0; y < original->height; y++) {
        for (int x = 0; x < original->width; x++) {
            int pixel_index = (y * original->width + x) * original->channels;
            
            unsigned char r = original->data[pixel_index];
            unsigned char g = original->data[pixel_index + 1];
            unsigned char b = original->data[pixel_index + 2];
        
            
            if (original->channels == 4) {
                unsigned char a = original->data[pixel_index + 3];
                if (a < 128) {
                    grayImg->data[y * original->width + x] = 0;
                } else {
                    if (method == GRAY_AVERAGE) {
                        grayImg->data[y * original->width + x] = (r + g + b) / 3.0f;
                    } else {
                        grayImg->data[y * original->width + x] = (0.2126f * r) + (0.7152 * g) + (0.0722 * b);
                    }
                }               
            } else {
                if (method == GRAY_AVERAGE) {
                    grayImg->data[y * original->width + x] = (r + g + b) / 3.0f;
                } else {
                    grayImg->data[y * original->width + x] = (0.2126f * r) + (0.7152 * g) + (0.0722 * b);
                }
            }
        }
    }

    return grayImg;
}


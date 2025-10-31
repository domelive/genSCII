#ifndef GENERATOR_H
#define GENERATOR_H

#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include "Dithering.h"
#include "../Image/Image.h"

typedef enum DitherMode {
    DITHER_NONE,
    DITHER_FLOYD_STEINBERG
} DitherMode;

typedef enum ColorMode {
    COLOR_NONE,
    COLOR_16,
    COLOR_256,
    COLOR_TRUE
} ColorMode;

typedef struct ASCIIGenConfig {
    const char* char_set;
    float terminal_aspect_ratio;
    bool use_average_pooling;
    GrayscaleMethod grayscale_method;
    ColorMode color_mode;
    DitherMode dither_mode;
} ASCIIGenConfig;

extern const ASCIIGenConfig DEFAULT_CONFIG;

// Generate ASCII from an already loaded image
// - Does NOT take ownership of `img`, so the caller must free it.
// - Writes output ti given FILE*
bool Generator_generateASCIIFromImage(Image* img, FILE* output, const ASCIIGenConfig* config);

// Loads image, generates ASCII and saves to file
bool Generator_generateACIIFromFile(const char* input_path, const char* output_path, const ASCIIGenConfig* config);

#endif // GENERATOR_H

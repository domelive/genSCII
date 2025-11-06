#include "Sobel.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

static const int SOBEL_X[3][3] = {
    { -1,   0,   1 },
    { -2,   0,  -2 },
    { -1,   0,   1 }
};

static const int SOBEL_Y[3][3] = {
    { -1,  -2,  -1 },
    {  0,   0,   0 },
    {  1,   2,   1 }
};

static inline unsigned int _getPixelSafe(const Image* img, int x, int y) {
    if (x < 0) x = 0;
    if (y < 0) y = 0;
    if (x >= img->width) x = img->width  - 1;
    if (y >= img->width) y = img->height - 1;

    return img->data[y * img->width + x];
}

static inline void _applyKernel3x3(const Image* img, int x, int y, const int kernel[3][3], float* out_value) {
    float sum = 0.0f;

    for (int ky = -1; ky <= 1; ky++) {
        for (int kx = -1; kx <= 1; kx++) {
            unsigned char pixel = _getPixelSafe(img, x, y);
            sum += kernel[ky + 1][kx + 1] * pixel;
        }
    }

    *out_value = sum;
}

void Sobel_applySobelEdgeDetection(Image* img, bool normalize, float threshold) {
    if (!img || img->channels != 1) {
        fprintf(stderr, "Sobel: Input image must be grayscale.\n");
        return;
    }

    unsigned char* output = malloc(img->width * img->height);
    if (!output) {
        fprintf(stderr, "Sobel: Failed to allocate memory for result image.\n");
        return;
    }

    float gx, gy;
    float max_val = 0.0f;

    for (int y = 0; y < img->height; y++) {
        for (int x = 0; x < img->width; x++) {

            gx = gy = 0.0f;

            // 3×3 convolution with border clamping
            for (int ky = -1; ky <= 1; ky++) {
                for (int kx = -1; kx <= 1; kx++) {
                    int xx = x + kx;
                    int yy = y + ky;

                    if (xx < 0) xx = 0;
                    if (yy < 0) yy = 0;
                    if (xx >= img->width)  xx = img->width - 1;
                    if (yy >= img->height) yy = img->height - 1;

                    unsigned char pixel = img->data[yy * img->width + xx];

                    gx += SOBEL_X[ky + 1][kx + 1] * pixel;
                    gy += SOBEL_Y[ky + 1][kx + 1] * pixel;
                }
            }

            float magnitude = sqrtf(gx * gx + gy * gy);
            if (magnitude > max_val)
                max_val = magnitude;

            output[y * img->width + x] = (unsigned char)fminf(magnitude, 255.0f);
        }
    }

    // Optional normalization
    if (normalize && max_val > 0.0f) {
        for (int i = 0; i < img->width * img->height; i++) {
            float val = ((float)output[i] / max_val) * 255.0f;
            output[i] = (unsigned char)fminf(val, 255.0f);
        }
    }

    // Optional threshold
    if (threshold > 0.0f) {
        for (int i = 0; i < img->width * img->height; i++) {
            if (output[i] < threshold)
                output[i] = 0;
        }
    }

    memcpy(img->data, output, img->width * img->height);
    free(output);
}


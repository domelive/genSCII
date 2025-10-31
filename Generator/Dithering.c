#include "Dithering.h"

static inline float _sampleCellLuminance(Image* img, int x0, int y0, int x1, int y1) {
    float total = 0.0f;
    int count = 0;

    for (int y = y0; y < y1; y++) {
        for (int x = x0; x < x1; x++) {
            total += img->data[y * img->width + x];
            count++;
        }
    }

    return (count > 0) ? total / count : 0.0f;
}

void Dithering_applyFloydSteinberg(Image* gray_img, 
                                   int ascii_width, int ascii_height, 
                                   float scale_x, float scale_y,
                                   const char* char_set)
{
    int len = strlen(char_set);
    if (len <= 1) return;

    float* buffer = malloc(ascii_width * ascii_height * sizeof(float));
    if (!buffer) {
        fprintf(stderr, "Dithering: failed to allocate buffer.\n");
        return;
    }

    for (int y = 0; y < ascii_height; y++) {
        for (int x = 0; x < ascii_width; x++) {
            int x0 = (int)(x * scale_x);
            int x1 = (int)((x + 1) * scale_x);
            int y0 = (int)(y * scale_y);
            int y1 = (int)((y + 1) * scale_y);

            buffer[y * ascii_width + x] = _sampleCellLuminance(gray_img, x0, y0, x1, y1);
        }
    }

    for (int y = 0; y < ascii_height; y++) {
        for (int x = 0; x < ascii_width; x++) {
            int idx = y * ascii_width + x;

            float old_brightness = buffer[idx];
            float norm = old_brightness / 255.0f;
            int char_idx = floorf(norm * (len - 1));
            float new_brightness = ((float)char_idx / (len - 1)) * 255.0f;

            buffer[idx] = new_brightness;

            float error = old_brightness - new_brightness;

            if (x + 1 < ascii_width) {
                int n = y * ascii_width + (x + 1);
                buffer[n] = fminf(255.0f, fmaxf(0.0f, buffer[n] + error * (7.0f / 16.0f)));
            }
            
            if (x - 1 >= 0 && y + 1 < ascii_height) {
                int n = (y + 1) * ascii_width + (x - 1);
                buffer[n] = fminf(255.0f, fmaxf(0.0f, buffer[n] + error * (3.0f / 16.0f)));
            }
            
            if (y + 1 < ascii_height) {
                int n = (y + 1) * ascii_width + x;
                buffer[n] = fminf(255.0f, fmaxf(0.0f, buffer[n] + error * (5.0f / 16.0f)));
            }
            
            if (x + 1 < ascii_width && y + 1 < ascii_height) {
                int n = (y + 1) * ascii_width + (x + 1);
                buffer[n] = fminf(255.0f, fmaxf(0.0f, buffer[n] + error * (1.0f / 16.0f)));
            }
        }
    }

    for (int y = 0; y < ascii_height; y++) {
        for (int x = 0; x < ascii_width; x++) {
            unsigned char q = (unsigned char)roundf(buffer[y * ascii_width + x]);

            int x0 = (int)(x * scale_x);
            int x1 = (int)((x + 1) * scale_x);
            int y0 = (int)(y * scale_y);
            int y1 = (int)((y + 1) * scale_y);

            for (int yy = y0; yy < y1; yy++) {
                for (int xx = x0; xx < x1; xx++) {
                    if (xx < gray_img->width && yy < gray_img->height)
                        gray_img->data[yy * gray_img->width + xx] = q;
                }
            }
        }
    }

    free(buffer);
}

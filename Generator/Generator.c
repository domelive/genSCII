#include "Generator.h"
#include <stdio.h>

static inline void _getTerminalDimensions(int* width, int* height) {
    struct winsize win;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &win);

    *width = win.ws_col;
    *height = win.ws_row;
}

static inline void _calculateASCIIDimensions(Image* img, float aspect_ratio, int* width, int* height, float* scale) {
    int term_width, term_height;
    _getTerminalDimensions(&term_width, &term_height);

    float scale_x = (float)img->width / term_width;
    float scale_y = (float)img->height / (term_height * aspect_ratio);
    float s = (scale_x > scale_y) ? scale_x : scale_y;

    *scale = s;
    *width = (int)(img->width / s);
    *height = (int)(img->height / (s * aspect_ratio));
}

static inline char _brightness2Char(float brightness) {
    float norm_brightness = brightness / 255.0f;

    int idx = (int)(norm_brightness * (strlen(CHAR_SET) - 1));
    if (idx < 0) idx = 0;
    if (idx >= (int)strlen(CHAR_SET)) idx = strlen(CHAR_SET) - 1;

    return CHAR_SET[idx];
}

void Generator_generateASCII(Image* img) {
    FILE* fd = fopen("generated_image.txt", "w");
    if (!fd) {
        fprintf(stderr, "Error opening file\n");
        exit(EXIT_FAILURE);       
    }

    Image* grayImg = Image_toGrayscale(img);

    float aspect_ratio = 2.0f;
    // fprintf(fd, "Image: %dx%d\n", img->width, img->height);
 
    float scale;
    int ascii_width, ascii_height;
    _calculateASCIIDimensions(img, aspect_ratio, &ascii_width, &ascii_height, &scale);
    // fprintf(fd, "ASCII: %dx%d (scale %.2f)\n", ascii_width, ascii_height, scale);

    // Nearest Neighbor Sampling
    // for (int y = 0; y < ascii_height; y++) {
    //     for (int x = 0; x < ascii_width; x++) {
    //         int pos_x = (int)((float)x * scale);
    //         int pos_y = (int)((float)y * scale * aspect_ratio);
    //
    //         if (pos_x >= img->width) pos_x = img->width - 1;
    //         if (pos_y >= img->height) pos_y = img->height - 1;
    //
    //         int idx = pos_y * img->width + pos_x;
    //         char c = _brightness2Char((float)grayImg->data[idx]);
    //
    //         fputc(c, fd);
    //     }
    //     fputc('\n', fd);
    // }

    // Average Pooling Sampling
    for (int y = 0; y < ascii_height; y++) {
        for (int x = 0; x < ascii_width; x++) {
            float total = 0.0f;
            int count = 0;

            int x_start = (int)(x * scale);
            int x_end = (int)((x + 1) * scale);
            int y_start = (int)(y * scale * aspect_ratio);
            int y_end = (int)((y + 1) * scale * aspect_ratio);

            // clamping values
            x_start = (x_start < 0) ? 0 : x_start;
            x_end = (x_end > img->width) ? img->width : x_end;
            y_start = (y_start < 0) ? 0 : y_start;
            y_end = (y_end > img->height) ? img->height : y_end;

            for (int yy = y_start; yy < y_end; yy++) {
                for (int xx = x_start; xx < x_end; xx++) {
                    total += grayImg->data[yy * img->width + xx];
                    count++;
                }
            }

            float avg_brightness = (count > 0) ? total / count : 0.0f;
            char c = _brightness2Char(avg_brightness);

            fputc(c, fd);
        }
        fputc('\n', fd);
    }

    fclose(fd);

    Image_free(grayImg);
}

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
    float scale_y = (float)img->height / term_height;
    float s = (scale_x > scale_y) ? scale_x : scale_y;

    *scale = s;
    *width = (int)(img->width / s);
    *height = (int)(img->height / (s* aspect_ratio));
}

static inline char _brightness2Char(float brightness) {
    float norm_brightness = brightness / 255.0f;

    int idx = (int)(norm_brightness * (strlen(CHAR_SET) - 1));
    if (idx < 0) idx = 0;
    if (idx >= (int)strlen(CHAR_SET)) idx = strlen(CHAR_SET) - 1;

    return CHAR_SET[idx];
}

void generateASCII(Image* img) {
    Image* grayImg = Image_toGrayscale(img);

    float aspect_ratio = 0.5f;
    printf("Image: %dx%d\n", img->width, img->height);
 
    float scale;
    int ascii_width, ascii_height;
    _calculateASCIIDimensions(img, aspect_ratio, &ascii_width, &ascii_height, &scale);
    printf("ASCII: %dx%d (scale %.2f)\n", ascii_width, ascii_height, scale);

    for (int y = 0; y < ascii_height; y++) {
        for (int x = 0; x < ascii_width; x++) {
            int pos_x = (int)((float)x * scale);
            int pos_y = (int)((float)y * scale * aspect_ratio);

            if (pos_x >= img->width) pos_x = img->width - 1;
            if (pos_y >= img->height) pos_y = img->height - 1;

            int idx = pos_y * img->width + pos_x;
            putchar(_brightness2Char((float)grayImg->data[idx]));
        }
        putchar('\n');
    }

    Image_free(grayImg);
    Image_free(img);
}

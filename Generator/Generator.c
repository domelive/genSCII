#include "Generator.h"
#include <stdio.h>

static inline void _getTerminalDimensions(int* width, int* height) {
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0) {
        *width = w.ws_col;
        *height = w.ws_row;
    } else {
        *width = 80;
        *height = 24;
    }
}

static inline char _brightness2Char(float brightness, const char* char_set) {
    float norm_brightness = brightness / 255.0f;

    int len = (int)strlen(char_set);
    int idx = (int)(norm_brightness * (len - 1));
    
    if (idx < 0) idx = 0;
    if (idx >= len) idx = len - 1;

    return char_set[idx];
}

static inline float _sampleRegion(Image* gray_img, int x0, int y0, int x1, int y1, bool use_avg) {
    // clamp values to image bounds
    x0 = (x0 < 0) ? 0 : x0;
    y0 = (y0 < 0) ? 0 : y0;
    x1 = (x1 > gray_img->width) ? gray_img->width : x1;
    y1 = (y1 > gray_img->height) ? gray_img->height : y1;

    if (x0 >= x1 || y0 > y1) return 0.0f;

    if (!use_avg)
        return (float)gray_img->data[y0 * gray_img->width + x0];

    int count = 0;
    float total = 0.0f;
    for (int yy = y0; yy < y1; yy++) {
        for (int xx = x0; xx < x1; xx++) {
            total += gray_img->data[yy * gray_img->width + xx];
            count++;
        }
    }

    return (count > 0) ? total / count : 0.0f;
}

static inline void _computeASCIIDims(Image* img, const ASCIIGenConfig* config,
                                     int term_width, int term_height,
                                     int* out_width, int* out_height,
                                     float* out_scale_x, float* out_scale_y) {
    // target ratio constrined by term_width and term_height
    float target_ratio = ((float)img->width / img->height) * config->terminal_aspect_ratio;
    
    // max posisble width and height in characters
    int max_width = term_width;
    int max_height = term_height;

    // try to fit by width first
    int w_by_width = max_width;
    int h_by_width = (int)((float)w_by_width / target_ratio);
    if (h_by_width > max_height) {
        // fit by height
        int h_by_height = max_height;
        int w_by_height = (int)(h_by_height * target_ratio);

        *out_width = w_by_height;
        *out_height = h_by_height;
    } else {
        *out_width = w_by_width;
        *out_height = h_by_width;
    }

    // avoid zero dimensions
    if (*out_width <= 0) *out_width = 1;
    if (*out_height <= 0) *out_height = 1;

    *out_scale_x = (float)img->width / *out_width;
    *out_scale_y = (float)img->height / *out_height;
}

static inline void _renderASCIIToFile(FILE* output, Image* gray_img, const ASCIIGenConfig* config,
                                      int ascii_width, int ascii_height, float scale_x, float scale_y) {
    for (int y = 0; y < ascii_height; y++) {
        for (int x = 0; x < ascii_width; x++) {
            int x0 = (int)(x * scale_x);
            int x1 = (int)((x + 1) * scale_x);
            int y0 = (int)(y * scale_y);
            int y1 = (int)((y + 1) * scale_y);

            float brightness = _sampleRegion(gray_img, x0, y0, x1, y1, config->use_average_pooling);
            
            char c = _brightness2Char(brightness, config->char_set);

            fputc(c, output);
        }
        fputc('\n', output);
    }
}

const ASCIIGenConfig DEFAULT_CONFIG = {
    .char_set = "@%#*+=-:. ",
    // .char_set = " .:-=+*#%@";
    // .char_set = "$@B%8&WM#*oahkbdpqwmZO0QLCJUYXzcvunxrjft/\\|()1{}[]?-_+~<>i!lI;:,\"^",
    .terminal_aspect_ratio = 2.0f,
    .use_average_pooling = true
};

bool Generator_generateASCIIFromImage(Image* img, FILE* output, const ASCIIGenConfig* config) {
    if (!img || !output) return false;
    const ASCIIGenConfig* cfg = config ? config : &DEFAULT_CONFIG;
    
    Image* gray = Image_toGrayscale(img);
    if (!gray) return false;

    int term_width, term_height;
    _getTerminalDimensions(&term_width, &term_height);

    int ascii_width, ascii_height;
    float scale_x, scale_y;
    _computeASCIIDims(img, cfg, term_width, term_height, &ascii_width, &ascii_height, &scale_x, &scale_y);

    _renderASCIIToFile(output, gray, cfg, ascii_width, ascii_height, scale_x, scale_y);

    Image_free(gray);
    
    return true;
}

bool Generator_generateACIIFromFile(const char* input_path, const char* output_path, const ASCIIGenConfig* config) {
    if (!input_path || !output_path) return false;

    Image* img = Image_load(input_path);
    if (!img) return false;

    FILE* out = fopen(output_path, "w");
    if (!out) {
        Image_free(img);
        return false;
    }

    bool success = Generator_generateASCIIFromImage(img, out, config);

    fclose(out);
    Image_free(img);

    return success;
}

#include "Generator.h"

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

static inline void _sampleRGBRegion(Image* rgb_img, int x0, int y0, int x1, int y1,
                                    bool use_avg,
                                    unsigned char* out_r,
                                    unsigned char* out_g,
                                    unsigned char* out_b) {
    x0 = (x0 < 0) ? 0 : x0;
    y0 = (y0 < 0) ? 0 : y0;
    x1 = (x1 > rgb_img->width) ? rgb_img->width : x1;
    y1 = (y1 > rgb_img->height) ? rgb_img->height : y1;
    
    if (x0 >= x1 || y0 > y1) {
        *out_r = *out_g = *out_b = 0;
        return;
    } 

    if (!use_avg) {
        int idx = y0 * rgb_img->width + x0;
        *out_r = rgb_img->data[idx * rgb_img->channels + 0];
        *out_g = rgb_img->data[idx * rgb_img->channels + 1];
        *out_b = rgb_img->data[idx * rgb_img->channels + 2];
        return;
    }

    int count = 0;
    long long sum_r = 0, sum_g = 0, sum_b = 0;
    for (int yy = y0; yy < y1; yy++) {
        for (int xx = x0; xx < x1; xx++) {
            int pixel_idx = yy * rgb_img->width + xx;
            sum_r += rgb_img->data[pixel_idx * rgb_img->channels + 0];
            sum_g += rgb_img->data[pixel_idx * rgb_img->channels + 1];
            sum_b += rgb_img->data[pixel_idx * rgb_img->channels + 2];
            count++;
        }
    }

    if (count > 0) {
        *out_r = (unsigned char)(sum_r / count);
        *out_g = (unsigned char)(sum_g / count);
        *out_b = (unsigned char)(sum_b / count);
    } else {
        *out_r = *out_g = *out_b = 0;
    }
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

static inline int _rgbToAnsi256(unsigned char r, unsigned char g, unsigned char b) {
    // convert each channel from 0-255 to a 0-5 scale
    int r6 = (r * 6) / 256;
    int g6 = (g * 6) / 256;
    int b6 = (b * 6) / 256;
    
    // clamp just for safety
    if (r6 > 5) r6 = 5;
    if (g6 > 5) g6 = 5;
    if (b6 > 5) b6 = 5;

    // compute and return index
    return 16 + (36 * r6) + (6 * g6) + b6;
}

static inline int _rgbToAnsi16(unsigned char r, unsigned char g, unsigned char b) {
    static const unsigned char ANSI16_RGB[16][3] = {
        {  0,   0,   0  }, { 128, 0,  0  }, { 0, 128,  0  }, { 128, 128,  0  },
        {  0 ,  0,  128 }, { 128, 0, 128 }, { 0, 128, 128 }, { 192, 192, 192 },
        { 128, 128, 128 }, { 255, 0,  0  }, { 0, 255,  0  }, { 255, 255,  0  },
        {  0,   0,  255 }, { 255, 0, 255 }, { 0, 255, 255 }, { 255, 255, 255 }
    };

    int best_index = 0;
    long long min_dist = LLONG_MAX;

    for (int i = 0; i < 16; i++) {
        long long dr = r - ANSI16_RGB[i][0];
        long long dg = g - ANSI16_RGB[i][1];
        long long db = b - ANSI16_RGB[i][2];

        long long dist = dr*dr + dg*dg + db*db;

        if (dist < min_dist) {
            min_dist = dist;
            best_index = i;
        }
    }

    return best_index;
}

static inline void _rgbToAnsiEscape(unsigned char r, unsigned char g, unsigned char b,
                                    ColorMode mode,
                                    char* out_buffer, size_t buffer_size) {
    switch (mode) {
        case COLOR_16: {
            int index = _rgbToAnsi16(r, g, b);
            snprintf(out_buffer, buffer_size, "38;5;%d", index); 
            break;
        }
        case COLOR_256: {
            int index = _rgbToAnsi256(r, g, b);
            snprintf(out_buffer, buffer_size, "38;5;%d", index); 
            break;
        }
        case COLOR_TRUE:
            snprintf(out_buffer, buffer_size, "38;2;%d;%d;%d", r, g, b); 
            break;
        default:
            out_buffer[0] = '\0';
    }
}

static inline void _renderASCIIToFile(FILE* output, 
                                      Image* render_img,   // grayscale or original
                                      Image* original_img, // always original RGB image
                                      const ASCIIGenConfig* config,
                                      int ascii_width, int ascii_height, 
                                      float scale_x, float scale_y) {
    for (int y = 0; y < ascii_height; y++) {
        for (int x = 0; x < ascii_width; x++) {
            int x0 = (int)(x * scale_x);
            int x1 = (int)((x + 1) * scale_x);
            int y0 = (int)(y * scale_y);
            int y1 = (int)((y + 1) * scale_y);
            
            float luminance;
            unsigned char avg_r = 0, avg_g = 0, avg_b = 0;

            if (config->color_mode == COLOR_NONE) {
                luminance = _sampleRegion(render_img, x0, y0, x1, y1, config->use_average_pooling);
            } else {
                _sampleRGBRegion(original_img, x0, y0, x1, y1, config->use_average_pooling, &avg_r, &avg_g, &avg_b);
                luminance = 0.2126f*avg_r + 0.7152f*avg_g + 0.0722f*avg_b;
            }

            char c = _brightness2Char(luminance, config->char_set);

            if (config->color_mode != COLOR_NONE) {
                char ansi_payload[32];
                _rgbToAnsiEscape(avg_r, avg_g, avg_b, config->color_mode, ansi_payload, sizeof(ansi_payload));
                fprintf(output, "\x1b[%sm%c\x1b[0m", ansi_payload, c);
            } else {
                fputc(c, output);
            }
        }
        fputc('\n', output);
    }
}

const ASCIIGenConfig DEFAULT_CONFIG = {
    .char_set = "$@B%8&WM#*oahkbdpqwmZO0QLCJUYXzcvunxrjft/\\|()1{}[]?-_+~i!lI;:,^'",
    .terminal_aspect_ratio = 2.0f,
    .use_average_pooling = true,
    .grayscale_method = GRAY_LUMINANCE,
    .color_mode = COLOR_NONE,
    .dither_mode = DITHER_NONE,
};

bool Generator_generateASCIIFromImage(Image* img, FILE* output, const ASCIIGenConfig* config) {
    if (!img || !output) return false;
    const ASCIIGenConfig* cfg = config ? config : &DEFAULT_CONFIG;
 
    int term_width, term_height;
    _getTerminalDimensions(&term_width, &term_height);

    int ascii_width, ascii_height;
    float scale_x, scale_y;
    _computeASCIIDims(img, cfg, term_width, term_height, &ascii_width, &ascii_height, &scale_x, &scale_y);
    
    Image* render_img = NULL;
    bool owns_render_img = false;

    if (cfg->color_mode == COLOR_NONE) {
        render_img = Image_toGrayscale(img, cfg->grayscale_method);
        owns_render_img = true;
    
        if (cfg->dither_mode == DITHER_FLOYD_STEINBERG) {
            Dithering_applyFloydSteinberg(render_img, ascii_width, ascii_height, scale_x, scale_y, cfg->char_set);
        }
    } else {
        render_img = img;
    }

    _renderASCIIToFile(output, render_img, img, cfg, ascii_width, ascii_height, scale_x, scale_y);

    if (owns_render_img) Image_free(render_img);
    
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

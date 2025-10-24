#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

// char set ordered from brightest to lightest
static const char* CHAR_SET = " .:-=+*#%@";

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: ./genSCII <img_path>");
        exit(EXIT_FAILURE);
    }

    const char* path = argv[1];
    int width, height, channels;

    unsigned char* img = stbi_load(path, &width, &height, &channels, 0);
    if (img == NULL) {
        perror("Error loading the image\n");
        exit(EXIT_FAILURE);
    }

    // Convert to grayscale (single channel)
    size_t gray_img_size = width * height;
    unsigned char* gray_img = malloc(gray_img_size);
    if (gray_img == NULL) {
        perror("Unable to allocate memory for the gray image.\n");
        stbi_image_free(img);
        exit(EXIT_FAILURE);
    }

    // Convert to grayscale - use only RGB channels, handle alpha
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int pixel_index = (y * width + x) * channels;
            unsigned char r = img[pixel_index];
            unsigned char g = img[pixel_index + 1];
            unsigned char b = img[pixel_index + 2];
            
            // If we have alpha channel and it's transparent, make it very dark (will become space)
            if (channels == 4) {
                unsigned char a = img[pixel_index + 3];
                if (a < 128) { // Consider it transparent if alpha is less than half
                    gray_img[y * width + x] = 0; // Make it black (will become space)
                } else {
                    gray_img[y * width + x] = (r + g + b) / 3.0f;
                }
            } else {
                gray_img[y * width + x] = (r + g + b) / 3.0f;
            }
        }
    }

    // Save grayscale image (optional, for debugging)
    // stbi_write_jpg("gray-image.jpg", width, height, 1, gray_img, 100);
    
    // Get terminal size
    struct winsize win;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &win);
    int term_width = win.ws_col;
    int term_height = win.ws_row;

    // Aspect ratio correction (typical terminal character aspect ratio is ~0.5)
    float aspect_ratio_correction = 0.5f;

    // Calculate scaling factors
    float scale_x = (float)width / term_width;
    float scale_y = (float)height / term_height;
    float scale = (scale_x > scale_y) ? scale_x : scale_y;

    // Calculate ASCII dimensions
    int ascii_width = (int)(width / scale);
    int ascii_height = (int)(height / (scale * aspect_ratio_correction));

    printf("Terminal: %d cols × %d rows\n", term_width, term_height);
    printf("Image: %dx%d\n", width, height);
    printf("ASCII: %dx%d (scale %.2f)\n", ascii_width, ascii_height, scale);

    // Calculate padding for centering
    int horizontal_padding = (term_width - ascii_width) / 2;
    int vertical_padding = (term_height - ascii_height) / 2;

    // Add vertical padding (top)
    for (int i = 0; i < vertical_padding && i < term_height; i++) {
        putchar('\n');
    }

    // Generate ASCII art
    for (int y = 0; y < ascii_height; y++) {
        // Add horizontal padding (left)
        for (int i = 0; i < horizontal_padding && i < term_width; i++) {
            putchar(' ');
        }
        
        for (int x = 0; x < ascii_width; x++) {
            // Map ASCII position back to original image coordinates
            int pos_x = (int)((float)x * scale);
            int pos_y = (int)((float)y * scale * aspect_ratio_correction);

            // Clamp to image boundaries
            if (pos_x >= width) pos_x = width - 1;
            if (pos_y >= height) pos_y = height - 1;

            // Get brightness value
            unsigned char brightness = gray_img[pos_y * width + pos_x];

            // Normalize and map to character set
            float norm_brightness = (float)brightness / 255.0f;
            int idx = (int)(norm_brightness * (strlen(CHAR_SET) - 1));
            
            if (idx < 0) idx = 0;
            if (idx >= strlen(CHAR_SET)) idx = strlen(CHAR_SET) - 1;

            char c = CHAR_SET[idx];
            putchar(c);
        }
        putchar('\n');
    }

    // Add vertical padding (bottom) if needed
    int remaining_vertical_padding = term_height - vertical_padding - ascii_height;
    for (int i = 0; i < remaining_vertical_padding && i < term_height; i++) {
        putchar('\n');
    }

    stbi_image_free(img);
    free(gray_img);
    return 0;
}

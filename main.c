#include <stdio.h>
#include <getopt.h>
#include <string.h>

#include "Generator/Generator.h"
#include "Image/Image.h"

static struct option long_options[] = {
    { "input",       required_argument, 0, 'i' },
    { "output",      required_argument, 0, 'o' },
    { "charset",     required_argument, 0, 'c' },
    { "aspect",      required_argument, 0, 'a' },
    { "gray-method", required_argument, 0, 'g' },
    { "colored",     required_argument, 0, 'm' },
    { "help",        no_argument,       0, 'h' },
    {0, 0, 0, 0}
};

int main(int argc, char* argv[]) {
    const char* input_path = NULL;
    const char* output_path = NULL;
    const char* char_set = DEFAULT_CONFIG.char_set;
    float aspect = DEFAULT_CONFIG.terminal_aspect_ratio;
    GrayscaleMethod method = DEFAULT_CONFIG.grayscale_method;
    ColorMode color = DEFAULT_CONFIG.color_mode;

    int opt;
    int long_index = 0;
    while ((opt = getopt_long(argc, argv, "i:o:c:a:g:m:h", long_options, &long_index)) != -1) {
        switch (opt) {
            case 'i':
                input_path = optarg;
                break;
            case 'o':
                output_path = optarg;
                break;
            case 'c':
                char_set = optarg;
                break;
            case 'a':
                aspect = atof(optarg);
                break;
            case 'g':
                if (strcmp(optarg, "average") == 0) {
                    method = GRAY_AVERAGE;
                } else if (strcmp(optarg, "luminance") == 0)  {
                    method = GRAY_LUMINANCE;
                } else { 
                    printf("%s is not a valid grayscale conversion method.\n", optarg); 
                    return 1; 
                }
                break;
            case 'm':
                if (strcmp(optarg, "16") == 0) {
                    color = COLOR_16;
                } else if (strcmp(optarg, "256") == 0) {
                    color = COLOR_256;
                } else if (strcmp(optarg, "true") == 0) {
                    color = COLOR_TRUE;
                } else {
                    printf("%s is not a valid color mode.\n", optarg); 
                    return 1;
                }
                break;
            case 'h':
                printf("Usage: %s [--input FILE] [--output FILE] [--charset SET] [--aspect RATIO] [--gray-method average|luminance]\n", argv[0]);
                return 0;
            default:
                fprintf(stderr, "Try '%s -h' for help.\n", argv[0]);
                return 1;
        }
    }

    if (!input_path || !output_path) {
        fprintf(stderr, "Error: --input and --output are required.\n");
        return -1;
    }

    ASCIIGenConfig cfg = DEFAULT_CONFIG;
    cfg.char_set = char_set;
    cfg.terminal_aspect_ratio = aspect;
    cfg.grayscale_method = method;
    cfg.color_mode = color;

    if (!Generator_generateACIIFromFile(input_path, output_path, &cfg)) {
        fprintf(stderr, "Failed to generate ASCII art.\n");
        return 1;
    }
    
    return 0;
}

#include <stdio.h>
#include <getopt.h>

#include "Generator/Generator.h"

static struct option long_options[] = {
    { "input",  required_argument, 0, 'i' },
    { "output", required_argument, 0, 'o' },
    { "chars",  required_argument, 0, 'c' },
    { "aspect", required_argument, 0, 'a' },
    { "help",   no_argument,       0, 'h' },
    {0, 0, 0, 0}
};

int main(int argc, char* argv[]) {
    const char* input_path = NULL;
    const char* output_path = NULL;
    const char* char_set = DEFAULT_CONFIG.char_set;
    float aspect = DEFAULT_CONFIG.terminal_aspect_ratio;

    int opt;
    int long_index = 0;
    while ((opt = getopt_long(argc, argv, "i:o:c:a:h", long_options, &long_index)) != -1) {
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
            case 'h':
                printf("Usage: %s --input FILE --output FILE [--chars SET] [--aspect RATIO]\n", argv[0]);
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


    if (!Generator_generateACIIFromFile(input_path, output_path, &cfg)) {
        fprintf(stderr, "Failed to generate ASCII art.\n");
        return 1;
    }
    
    return 0;
}

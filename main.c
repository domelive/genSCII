#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include "Image/Image.h"
#include "Generator/Generator.h"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: ./genSCII <img_path>");
        exit(EXIT_FAILURE);
    }

    const char* path = argv[1];

    Image* img = Image_load(path);    
    if (!img) {
        exit(EXIT_FAILURE);
    }

    Generator_generateASCII(img);

    Image_free(img);
    return 0;
}

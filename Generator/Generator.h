#ifndef GENERATOR_H
#define GENERATOR_H

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include "../Image/Image.h"

static const char* CHAR_SET = "@%#*+=-:. ";
// static const char* CHAR_SET = " .:-=+*#%@";
// static const char* CHAR_SET = "$@B%8&WM#*oahkbdpqwmZO0QLCJUYXzcvunxrjft/\\|()1{}[]?-_+~<>i!lI;:,\"^";

static inline void _getTerminalDimensions(int* width, int* height);
static inline void _calculateASCIIDimensions(Image* img, float aspect_ratio, int* width, int* height, float* scale);
static inline char _brightness2Char(float brightness);

void Generator_generateASCII(Image* img);

#endif // GENERATOR_H

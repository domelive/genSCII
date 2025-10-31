#ifndef DITHERING_H
#define DITHERING_H

#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#include "../Image/Image.h"

void Dithering_applyFloydSteinberg(Image* gray_img,
                                   int ascii_width, int ascii_height,
                                   float scale_x, float scale_y,
                                   const char* char_set);

#endif // DITHERING_H

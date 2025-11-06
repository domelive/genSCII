#ifndef SOBEL_H
#define SOBEL_H

#include <stdio.h>

#include "../Image/Image.h"

void Sobel_applySobelEdgeDetection(Image* img, bool normalize, float threshold);

#endif // SOBEL.H

#include "util.h"

/* Function to clamp d between the limits min and max */
float clamp(float d, float min, float max) {
    if (d < min) return min;
    if (d > max) return max;
    return d;
}

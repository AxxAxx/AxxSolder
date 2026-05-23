#ifndef UTIL_H
#define UTIL_H

/* Small math/utility helpers used across modules.
 *
 * If this header grows past a handful of functions, consider splitting
 * by category (math, string, etc.). For now everything fits comfortably
 * in one file. */

/* Clamp `d` to the inclusive range [min, max]. */
float clamp(float d, float min, float max);

#endif /* UTIL_H */

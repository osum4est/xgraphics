#include "xgraphics_utils.h"

#include <cmath>

float xgraphics_utils::srgb_to_linear(float x) {
    if (x <= 0.0f) return 0.0f;
    else if (x >= 1.0f) return 1.0f;
    else if (x < 0.04045f) return x / 12.92f;
    else return powf((x + 0.055f) / 1.055f, 2.4f);
}

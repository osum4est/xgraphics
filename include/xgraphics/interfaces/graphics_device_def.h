#ifndef WPEX_GRAPHICS_DEVICE_DEF_H
#define WPEX_GRAPHICS_DEVICE_DEF_H

#include <string>

enum class device_type { integrated, discrete, other };

struct graphics_device_def {
    device_type type;
    std::string name;

    graphics_device_def() = default;
    graphics_device_def(const graphics_device_def&) = delete;
};

#endif

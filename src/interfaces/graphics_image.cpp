#include "xgraphics/interfaces/graphics_image.h"

graphics_image::graphics_image(uint32_t width, uint32_t height, graphics_image_format format)
    : _width(width), _height(height), _format(format) { }

uint32_t graphics_image::width() const {
    return _width;
}

uint32_t graphics_image::height() const {
    return _height;
}

graphics_image_format graphics_image::format() const {
    return _format;
}
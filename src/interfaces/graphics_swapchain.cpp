#include "xgraphics/interfaces/graphics_swapchain.h"

graphics_swapchain::graphics_swapchain(uint32_t width, uint32_t height) : _width(width), _height(height) { }

uint32_t graphics_swapchain::width() const {
    return _width;
}

uint32_t graphics_swapchain::height() const {
    return _height;
}

void graphics_swapchain::resize(uint32_t width, uint32_t height) {
    _width = width;
    _height = height;
}

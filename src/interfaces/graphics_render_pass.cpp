#include "xgraphics/interfaces/graphics_render_pass.h"

#include <cstdint>

graphics_render_pass::graphics_render_pass(const graphics_swapchain& swapchain) : _swapchain(swapchain) { }

void graphics_render_pass::set_clear_color(uint32_t clear_color) {
    _clear_color = clear_color;
}

const graphics_swapchain& graphics_render_pass::swapchain() const {
    return _swapchain;
}

uint32_t graphics_render_pass::clear_color() const {
    return _clear_color;
}

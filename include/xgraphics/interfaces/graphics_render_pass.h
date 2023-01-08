#ifndef WPEX_GRAPHICS_RENDER_PASS_H
#define WPEX_GRAPHICS_RENDER_PASS_H

#include "graphics_swapchain.h"
#include <cstdint>

// TODO: Rename these to xgraphics_render_pass??
class graphics_render_pass {
    const graphics_swapchain& _swapchain;
    uint32_t _clear_color = 0;

  protected:
    // TODO: Take in attachment descriptions/support subpasses
    explicit graphics_render_pass(const graphics_swapchain& swapchain);

  public:
    graphics_render_pass(const graphics_render_pass&) = delete;
    virtual ~graphics_render_pass() = default;

    virtual void set_clear_color(uint32_t clear_color);

    [[nodiscard]] const graphics_swapchain& swapchain() const;
    [[nodiscard]] uint32_t clear_color() const;
};

#endif

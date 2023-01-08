#ifndef WPEX_GRAPHICS_SWAPCHAIN_H
#define WPEX_GRAPHICS_SWAPCHAIN_H

#include <cstdint>

class graphics_swapchain {
    uint32_t _width;
    uint32_t _height;

  protected:
    graphics_swapchain(uint32_t width, uint32_t height);

  public:
    graphics_swapchain(const graphics_swapchain&) = delete;
    virtual ~graphics_swapchain() = default;

    [[nodiscard]] uint32_t width() const;
    [[nodiscard]] uint32_t height() const;

    virtual void swap() = 0;
    virtual void resize(uint32_t width, uint32_t height);
};

#endif

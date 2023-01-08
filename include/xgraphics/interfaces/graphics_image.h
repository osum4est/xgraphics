#ifndef WPEX_GRAPHICS_IMAGE_H
#define WPEX_GRAPHICS_IMAGE_H

#include <cstdint>

enum class graphics_image_format {
    rgba_8_srgb,
    rgba_8_unorm,
};

class graphics_image {
    uint32_t _width;
    uint32_t _height;
    graphics_image_format _format;

  protected:
    explicit graphics_image(uint32_t width, uint32_t height, graphics_image_format format);

  public:
    graphics_image(const graphics_image&) = delete;
    virtual ~graphics_image() = default;

    virtual void write(const void* data, uint32_t size) = 0;

    [[nodiscard]] uint32_t width() const;
    [[nodiscard]] uint32_t height() const;
    [[nodiscard]] graphics_image_format format() const;
};

#endif

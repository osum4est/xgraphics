#ifndef XGRAPHICS_METAL_IMAGE_H
#define XGRAPHICS_METAL_IMAGE_H

#import <Metal/Metal.h>
#import <result/result.h>
#import <xgraphics/interfaces/graphics_image.h>

class metal_image : public graphics_image {
    id<MTLTexture> _texture;

    metal_image(uint32_t width, uint32_t height, graphics_image_format format, id<MTLTexture> texture);

  public:
    static result::ptr<graphics_image> create(uint32_t width, uint32_t height, graphics_image_format format,
                                              id<MTLDevice> device);

    [[nodiscard]] id<MTLTexture> texture() const;

    void write(const void* data, uint32_t size) override;
};

#endif

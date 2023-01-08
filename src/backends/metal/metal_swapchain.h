#ifndef XGRAPHICS_METAL_SWAPCHAIN_H
#define XGRAPHICS_METAL_SWAPCHAIN_H

#import <MetalKit/MetalKit.h>
#import <result/result.h>
#import <xgraphics/xgraphics.h>

class metal_swapchain : public graphics_swapchain {
    CAMetalLayer* _layer;
    id<CAMetalDrawable> _current_drawable = nullptr;
    id<MTLTexture> _depth_stencil_texture;

    explicit metal_swapchain(CAMetalLayer* layer, uint32_t width, uint32_t height,
                             id<MTLTexture> depth_stencil_texture);

  public:
    static result::ptr<graphics_swapchain> create(id<MTLDevice> device, CAMetalLayer* layer, uint32_t width,
                                                  uint32_t height);

    [[nodiscard]] id<CAMetalDrawable> current_drawable() const;
    [[nodiscard]] id<MTLTexture> depth_stencil_texture() const;

    void swap() override;
    void resize(uint32_t width, uint32_t height) override;

  private:
    static id<MTLTexture> create_depth_stencil_texture(id<MTLDevice> device, uint32_t width, uint32_t height);
};

#endif

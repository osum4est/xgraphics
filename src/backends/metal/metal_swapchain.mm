#include "metal_swapchain.h"

metal_swapchain::metal_swapchain(CAMetalLayer* layer, uint32_t width, uint32_t height,
                                 id<MTLTexture> depth_stencil_texture)
    : graphics_swapchain(width, height), _layer(layer), _depth_stencil_texture(depth_stencil_texture) { }

result::ptr<graphics_swapchain> metal_swapchain::create(id<MTLDevice> device, CAMetalLayer* layer, uint32_t width,
                                                        uint32_t height) {
    layer.pixelFormat = MTLPixelFormatBGRA8Unorm_sRGB;
    layer.drawableSize = CGSizeMake(width, height);

    auto depth_stencil_texture = create_depth_stencil_texture(device, width, height);
    return result::ok(new metal_swapchain(layer, width, height, depth_stencil_texture));
}

id<CAMetalDrawable> metal_swapchain::current_drawable() const {
    return _current_drawable;
}

id<MTLTexture> metal_swapchain::depth_stencil_texture() const {
    return _depth_stencil_texture;
}

void metal_swapchain::swap() {
    _current_drawable = [_layer nextDrawable];
}

void metal_swapchain::resize(uint32_t width, uint32_t height) {
    graphics_swapchain::resize(width, height);
    _layer.drawableSize = CGSizeMake(width, height);
    _depth_stencil_texture = create_depth_stencil_texture(_layer.device, width, height);
}

id<MTLTexture> metal_swapchain::create_depth_stencil_texture(id<MTLDevice> device, uint32_t width, uint32_t height) {
    auto depth_descriptor = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatDepth32Float
                                                                               width:width
                                                                              height:height
                                                                           mipmapped:NO];
    depth_descriptor.usage = MTLTextureUsageRenderTarget;
    return [device newTextureWithDescriptor:depth_descriptor];
}

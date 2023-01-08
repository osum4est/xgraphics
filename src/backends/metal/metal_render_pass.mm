#include "metal_render_pass.h"
#import "../common/xgraphics_utils.h"

metal_render_pass::metal_render_pass(const metal_swapchain& swapchain, MTLRenderPassDescriptor* render_pass_descriptor)
    : graphics_render_pass(swapchain), _render_pass_descriptor(render_pass_descriptor) { }

result::ptr<graphics_render_pass> metal_render_pass::create(const metal_swapchain& swapchain) {
    MTLRenderPassDescriptor* render_pass_descriptor = [MTLRenderPassDescriptor renderPassDescriptor];
    render_pass_descriptor.colorAttachments[0].loadAction = MTLLoadActionClear;
    render_pass_descriptor.colorAttachments[0].storeAction = MTLStoreActionStore;

    render_pass_descriptor.depthAttachment.loadAction = MTLLoadActionClear;
    render_pass_descriptor.depthAttachment.storeAction = MTLStoreActionStore;
    render_pass_descriptor.depthAttachment.clearDepth = 1.0f;

    return result::ok(new metal_render_pass(swapchain, render_pass_descriptor));
}

void metal_render_pass::set_clear_color(uint32_t clear_color) {
    graphics_render_pass::set_clear_color(clear_color);

    _render_pass_descriptor.colorAttachments[0].clearColor =
        MTLClearColorMake(xgraphics_utils::srgb_to_linear((float) ((clear_color >> 16) & 0xFF) / 255.0f),
                          xgraphics_utils::srgb_to_linear((float) ((clear_color >> 8) & 0xFF) / 255.0f),
                          xgraphics_utils::srgb_to_linear((float) (clear_color & 0xFF) / 255.0f),
                          (float) ((clear_color >> 24) & 0xFF) / 255.0f);
}

MTLRenderPassDescriptor* metal_render_pass::render_pass_descriptor() const {
    return _render_pass_descriptor;
}

#ifndef XGRAPHICS_METAL_RENDER_PASS_H
#define XGRAPHICS_METAL_RENDER_PASS_H

#import "metal_swapchain.h"
#import <Metal/Metal.h>
#import <result/result.h>
#import <xgraphics/interfaces/graphics_render_pass.h>

class metal_render_pass : public graphics_render_pass {
    MTLRenderPassDescriptor* _render_pass_descriptor;

    explicit metal_render_pass(const metal_swapchain& swapchain, MTLRenderPassDescriptor* render_pass_descriptor);

  public:
    static result::ptr<graphics_render_pass> create(const metal_swapchain& swapchain);

    void set_clear_color(uint32_t clear_color) override;

    [[nodiscard]] MTLRenderPassDescriptor* render_pass_descriptor() const;
};

#endif

#ifndef XGRAPHICS_METAL_DEVICE_H
#define XGRAPHICS_METAL_DEVICE_H

#import "metal_sync_context.h"
#import <MetalKit/MetalKit.h>
#import <result/result.h>
#import <xgraphics/xgraphics.h>

class metal_device : public graphics_device {
    id<MTLDevice> _device;
    id<MTLCommandQueue> _command_queue;
    id<MTLCommandBuffer> _command_buffer_to_present = nullptr;
    CAMetalLayer* _layer;
    std::unique_ptr<metal_sync_context> _sync_context;

    // TODO: Make these constructors private
    explicit metal_device(std::unique_ptr<graphics_device_def> def, const graphics_config& config, id<MTLDevice> device,
                          id<MTLCommandQueue> command_queue, CAMetalLayer* layer,
                          std::unique_ptr<metal_sync_context> sync_context);

  protected:
    void wait_for_frame() override;
    void frame_changed(int current_frame) override;

  public:
    static result::ptr<graphics_device> create(std::unique_ptr<graphics_device_def> def, const graphics_config& config,
                                               CAMetalLayer* layer);

    result::ptr<graphics_swapchain> create_swapchain(uint32_t width, uint32_t height) override;
    result::ptr<graphics_shader> create_shader(std::unique_ptr<shader_binary> binary) override;
    result::ptr<graphics_render_pass> create_render_pass(const graphics_swapchain& swapchain) override;
    result::ptr<graphics_resource_layout>
    create_resource_layout(const std::vector<const graphics_shader*>& stages) override;
    result::ptr<graphics_resource_set> create_resource_set(const graphics_resource_layout& layout,
                                                           resource_set_ref ref) override;
    result::ptr<graphics_pipeline> create_pipeline(const graphics_pipeline_init& init) override;
    result::ptr<graphics_command_buffer> create_command_buffer() override;
    result::ptr<graphics_buffer> create_buffer(buffer_usage_flags usage, uint32_t size) override;
    result::ptr<graphics_image> create_image(uint32_t width, uint32_t height, graphics_image_format format) override;
    result::ptr<graphics_sampler> create_sampler(const graphics_sampler_init& init) override;
    result::ptr<graphics_uniform_buffer> create_uniform_buffer(const shader_variable_type& type) override;

    void submit_command_buffer(const graphics_command_buffer& command_buffer) override;
    void present(graphics_swapchain& swapchain) override;
};

#endif

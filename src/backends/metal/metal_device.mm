#import "metal_device.h"

#import "metal_buffer.h"
#import "metal_command_buffer.h"
#import "metal_device_def.h"
#import "metal_image.h"
#import "metal_pipeline.h"
#import "metal_render_pass.h"
#import "metal_resource_layout.h"
#import "metal_resource_set.h"
#import "metal_sampler.h"
#import "metal_shader.h"
#import "metal_uniform_buffer.h"

metal_device::metal_device(std::unique_ptr<graphics_device_def> def, const graphics_config& config,
                           id<MTLDevice> device, id<MTLCommandQueue> command_queue, CAMetalLayer* layer,
                           std::unique_ptr<metal_sync_context> sync_context)
    : graphics_device(std::move(def), config),
      _device(device),
      _command_queue(command_queue),
      _layer(layer),
      _sync_context(std::move(sync_context)) { }

void metal_device::wait_for_frame() {
    dispatch_semaphore_wait(_sync_context->semaphore(), DISPATCH_TIME_FOREVER);
}

void metal_device::frame_changed(int current_frame) {
    _sync_context->set_current_frame(current_frame);
}

result::ptr<graphics_device> metal_device::create(std::unique_ptr<graphics_device_def> def,
                                                  const graphics_config& config, CAMetalLayer* layer) {
    const auto& native_def = (const metal_device_def&) *def;
    id<MTLCommandQueue> command_queue = [native_def.metal_device newCommandQueue];

    auto sync_context = GET_OR_FORWARD(metal_sync_context::create(config));
    return result::ok(new metal_device(std::move(def), config, native_def.metal_device, command_queue, layer,
                                       std::move(sync_context)));
}

result::ptr<graphics_swapchain> metal_device::create_swapchain(uint32_t width, uint32_t height) {
    return metal_swapchain::create(_device, _layer, width, height);
}

result::ptr<graphics_shader> metal_device::create_shader(std::unique_ptr<shader_binary> binary) {
    return metal_shader::create(std::move(binary), _device);
}

result::ptr<graphics_render_pass> metal_device::create_render_pass(const graphics_swapchain& swapchain) {
    const auto& native_swapchain = (const metal_swapchain&) swapchain;
    return metal_render_pass::create(native_swapchain);
}

result::ptr<graphics_resource_layout>
metal_device::create_resource_layout(const std::vector<const graphics_shader*>& stages) {
    return metal_resource_layout::create(stages);
}

result::ptr<graphics_resource_set> metal_device::create_resource_set(const graphics_resource_layout& layout,
                                                                     resource_set_ref ref) {
    const auto& native_layout = (const metal_resource_layout&) layout;
    return metal_resource_set::create(native_layout, ref, _device, *_sync_context);
}

result::ptr<graphics_pipeline> metal_device::create_pipeline(const graphics_pipeline_init& init) {
    return metal_pipeline::create(init, _device);
}

result::ptr<graphics_command_buffer> metal_device::create_command_buffer() {
    return metal_command_buffer::create(_command_queue);
}

result::ptr<graphics_buffer> metal_device::create_buffer(buffer_usage_flags usage, uint32_t size) {
    return metal_buffer::create(usage, size, _device);
}

result::ptr<graphics_image> metal_device::create_image(uint32_t width, uint32_t height, graphics_image_format format) {
    return metal_image::create(width, height, format, _device);
}

result::ptr<graphics_sampler> metal_device::create_sampler(const graphics_sampler_init& init) {
    return metal_sampler::create(init, _device);
}

result::ptr<graphics_uniform_buffer> metal_device::create_uniform_buffer(const shader_variable_type& type) {
    return metal_uniform_buffer::create(type, _device, *_sync_context);
}

void metal_device::submit_command_buffer(const graphics_command_buffer& command_buffer) {
    const auto& native_command_buffer = (const metal_command_buffer&) command_buffer;
    _command_buffer_to_present = native_command_buffer.command_buffer();
}

void metal_device::present(graphics_swapchain& swapchain) {
    auto& native_swapchain = (metal_swapchain&) swapchain;
    [_command_buffer_to_present presentDrawable:native_swapchain.current_drawable()];

    [_command_buffer_to_present addCompletedHandler:^(id<MTLCommandBuffer> command_buffer) {
      dispatch_semaphore_signal(_sync_context->semaphore());
    }];

    [_command_buffer_to_present commit];
    _command_buffer_to_present = nullptr;
}

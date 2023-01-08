#ifndef WPEX_GRAPHICS_DEVICE_H
#define WPEX_GRAPHICS_DEVICE_H

#include "graphics_buffer.h"
#include "graphics_command_buffer.h"
#include "graphics_device_def.h"
#include "graphics_pipeline.h"
#include "graphics_render_pass.h"
#include "graphics_resource_layout.h"
#include "graphics_shader.h"
#include "graphics_swapchain.h"
#include <result/result.h>
#include <xgraphics/graphics_config.h>
#include <xgraphics/shaders/shader_binary.h>

class graphics_device {
    std::unique_ptr<graphics_device_def> _def;
    graphics_config _config;
    int _current_frame = 0;

  protected:
    explicit graphics_device(std::unique_ptr<graphics_device_def> def, const graphics_config& config);
    virtual void wait_for_frame() = 0;
    virtual void frame_changed(int current_frame);

  public:
    graphics_device(const graphics_device&) = delete;
    virtual ~graphics_device() = default;

    [[nodiscard]] const graphics_device_def& def() const;
    [[nodiscard]] int current_frame() const;

    void advance_frame();

    // TODO: create_surface -> graphics_surface -> graphics_surface.create_swapchain() instead?
    virtual result::ptr<graphics_swapchain> create_swapchain(uint32_t width, uint32_t height) = 0;
    virtual result::ptr<graphics_shader> create_shader(std::unique_ptr<shader_binary> binary) = 0;
    virtual result::ptr<graphics_render_pass> create_render_pass(const graphics_swapchain& swapchain) = 0;
    virtual result::ptr<graphics_resource_layout>
    create_resource_layout(const std::vector<const graphics_shader*>& stages) = 0;
    virtual result::ptr<graphics_resource_set> create_resource_set(const graphics_resource_layout& layout,
                                                                   resource_set_ref set) = 0;
    virtual result::ptr<graphics_pipeline> create_pipeline(const graphics_pipeline_init& init) = 0;
    virtual result::ptr<graphics_buffer> create_buffer(buffer_usage_flags usage, uint32_t size) = 0;
    virtual result::ptr<graphics_image> create_image(uint32_t width, uint32_t height, graphics_image_format format) = 0;
    virtual result::ptr<graphics_sampler> create_sampler(const graphics_sampler_init& init) = 0;
    virtual result::ptr<graphics_uniform_buffer> create_uniform_buffer(const shader_variable_type& type) = 0;
    virtual result::ptr<graphics_command_buffer> create_command_buffer() = 0;

    virtual void submit_command_buffer(const graphics_command_buffer& command_buffer) = 0;
    virtual void present(graphics_swapchain& swapchain) = 0;
};

#endif

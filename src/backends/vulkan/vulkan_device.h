#ifndef XGRAPHICS_VULKAN_DEVICE_H
#define XGRAPHICS_VULKAN_DEVICE_H

#include "vulkan_memory_context.h"
#include "vulkan_sync_context.h"
#include <result/result.h>
#include <vector>
#include <vulkan/vulkan.h>
#include <xgraphics/xgraphics.h>

struct vulkan_device_init {
    VkInstance instance;
    std::unique_ptr<graphics_device_def> def;
    const graphics_config& config;
    VkSurfaceKHR surface;
};

struct vulkan_device_state {
    VkDevice device;
    VkQueue graphics_queue;
    VkQueue present_queue;
    VkQueue transfer_queue;
    VkCommandPool command_pool;
    VkCommandPool transfer_command_pool;
    std::unique_ptr<vulkan_sync_context> sync_context;
    std::unique_ptr<vulkan_memory_context> memory_context;
};

class vulkan_device : public graphics_device {
    VkDevice _device;
    VkSurfaceKHR _surface;
    VkQueue _graphics_queue;
    VkQueue _present_queue;
    VkQueue _transfer_queue;
    VkCommandPool _command_pool;
    VkCommandPool _transfer_command_pool;
    std::unique_ptr<vulkan_sync_context> _sync_context;
    std::unique_ptr<vulkan_memory_context> _memory_context;

    const static std::vector<const char*> REQUIRED_EXTENSIONS;

    explicit vulkan_device(vulkan_device_init& init, vulkan_device_state& state);

  protected:
    void wait_for_frame() override;
    void frame_changed(int current_frame) override;

  public:
    ~vulkan_device() override;

    static result::ptr<graphics_device_def> create_def(VkPhysicalDevice physical_device, VkSurfaceKHR surface);
    static result::ptr<graphics_device> create(vulkan_device_init& init);

    result::ptr<graphics_swapchain> create_swapchain(uint32_t width, uint32_t height) override;
    result::ptr<graphics_shader> create_shader(std::unique_ptr<shader_binary> binary) override;
    result::ptr<graphics_render_pass> create_render_pass(const graphics_swapchain& swapchain) override;
    result::ptr<graphics_resource_layout>
    create_resource_layout(const std::vector<const graphics_shader*>& stages) override;
    result::ptr<graphics_resource_set> create_resource_set(const graphics_resource_layout& layout,
                                                           resource_set_ref ref) override;
    result::ptr<graphics_pipeline> create_pipeline(const graphics_pipeline_init& init) override;
    result::ptr<graphics_buffer> create_buffer(buffer_usage_flags usage, uint32_t size) override;
    result::ptr<graphics_image> create_image(uint32_t width, uint32_t height, graphics_image_format format) override;
    result::ptr<graphics_sampler> create_sampler(const graphics_sampler_init& init) override;
    result::ptr<graphics_uniform_buffer> create_uniform_buffer(const shader_variable_type& type) override;
    result::ptr<graphics_command_buffer> create_command_buffer() override;

    void submit_command_buffer(const graphics_command_buffer& command_buffer) override;
    void present(graphics_swapchain& swapchain) override;
};

#endif
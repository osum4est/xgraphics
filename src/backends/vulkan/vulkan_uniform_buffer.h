#ifndef XGRAPHICS_VULKAN_UNIFORM_BUFFER_H
#define XGRAPHICS_VULKAN_UNIFORM_BUFFER_H

#include "vulkan_memory_context.h"
#include "vulkan_sync_context.h"
#include <xgraphics/interfaces/graphics_uniform_buffer.h>

struct vulkan_uniform_buffer_buffer {
    VkBuffer buffer;
    void* mapped_data;
};

class vulkan_uniform_buffer : public graphics_uniform_buffer {
    VkDevice _device;
    vulkan_sync_context* _sync_context;
    vulkan_memory_context* _memory_context;
    std::vector<vulkan_uniform_buffer_buffer> _buffers;

    vulkan_uniform_buffer(const shader_variable_type& type, uint32_t size, VkDevice device,
                          vulkan_sync_context* sync_context, vulkan_memory_context* memory_context,
                          const std::vector<vulkan_uniform_buffer_buffer>& buffers);

  protected:
    void set_data(const shader_variable_type& type, uint32_t offset, uint32_t size, const void* data) override;

  public:
    ~vulkan_uniform_buffer() override;

    static result::ptr<graphics_uniform_buffer> create(const shader_variable_type& type, VkDevice device,
                                                       vulkan_sync_context& sync_context,
                                                       vulkan_memory_context& memory_context);

    [[nodiscard]] VkBuffer buffer(uint32_t frame) const;
};

#endif

#ifndef XGRAPHICS_VULKAN_BUFFER_H
#define XGRAPHICS_VULKAN_BUFFER_H

#include "vulkan_device_def.h"
#include "vulkan_memory_context.h"
#include <xgraphics/interfaces/graphics_buffer.h>

struct vulkan_buffer_init {
    VkDevice device;
    buffer_usage_flags usage;
    size_t size;
    const vulkan_device_def& def;
    vulkan_memory_context& memory_context;
    VkCommandPool transfer_command_pool;
    VkQueue transfer_queue;
};

class vulkan_buffer : public graphics_buffer {
    VkDevice _device;
    vulkan_memory_context& _memory_context;
    VkBuffer _staging_buffer;
    VkBuffer _buffer;
    VkCommandPool _transfer_command_pool;
    VkQueue _transfer_queue;

    vulkan_buffer(const vulkan_buffer_init& init, VkBuffer staging_buffer, VkBuffer buffer);

  public:
    ~vulkan_buffer() override;

    static result::ptr<graphics_buffer> create(const vulkan_buffer_init& init);

    [[nodiscard]] VkBuffer buffer() const;

    void write(const void* data, uint32_t size) override;
};

#endif

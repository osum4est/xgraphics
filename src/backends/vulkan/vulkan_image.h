#ifndef XGRAPHICS_VULKAN_IMAGE_H
#define XGRAPHICS_VULKAN_IMAGE_H

#include "vulkan_device_def.h"
#include "vulkan_memory_context.h"
#include <result/result.h>
#include <vulkan/vulkan.h>
#include <xgraphics/interfaces/graphics_image.h>

struct vulkan_image_init {
    uint32_t width;
    uint32_t height;
    graphics_image_format format;
    VkDevice device;
    const vulkan_device_def* def;
    vulkan_memory_context* memory_context;
    VkCommandPool transfer_command_pool;
    VkQueue transfer_queue;
};

class vulkan_image : public graphics_image {
    VkDevice _device;
    vulkan_memory_context* _memory_context;
    VkCommandPool _transfer_command_pool;
    VkQueue _transfer_queue;

    VkBuffer _staging_buffer;
    VkImage _image;
    VkImageView _image_view;

    vulkan_image(const vulkan_image_init& init, VkBuffer staging_buffer, VkImage image, VkImageView image_view);

  public:
    ~vulkan_image() override;

    static result::ptr<graphics_image> create(const vulkan_image_init& init);

    void write(const void* data, uint32_t size) override;

    [[nodiscard]] VkImage image() const;
    [[nodiscard]] VkImageView image_view() const;
};

#endif

#include "vulkan_image.h"

vulkan_image::vulkan_image(const vulkan_image_init& init, VkBuffer staging_buffer, VkImage image,
                           VkImageView image_view)
    : graphics_image(init.width, init.height, init.format),
      _device(init.device),
      _memory_context(init.memory_context),
      _transfer_command_pool(init.transfer_command_pool),
      _transfer_queue(init.transfer_queue),
      _staging_buffer(staging_buffer),
      _image(image),
      _image_view(image_view) { }

vulkan_image::~vulkan_image() {
    _memory_context->destroy_buffer(_staging_buffer);
    _memory_context->destroy_image(_image);
}

result::ptr<graphics_image> vulkan_image::create(const vulkan_image_init& init) {
    int channels = 0;
    VkFormat format = VK_FORMAT_UNDEFINED;

    switch (init.format) {
        case graphics_image_format::rgba_8_srgb:
            channels = 4;
            format = VK_FORMAT_R8G8B8A8_SRGB;
            break;
        case graphics_image_format::rgba_8_unorm:
            channels = 4;
            format = VK_FORMAT_R8G8B8A8_UNORM;
            break;
    }

    VkDeviceSize image_size = init.width * init.height * channels;
    VkBufferCreateInfo staging_buffer_info = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = image_size,
        .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = nullptr,
    };

    if (init.def->transfer_family != init.def->graphics_family) {
        uint32_t queue_family_indices[] = {init.def->transfer_family.value(), init.def->graphics_family.value()};
        staging_buffer_info.sharingMode = VK_SHARING_MODE_CONCURRENT;
        staging_buffer_info.queueFamilyIndexCount = 2;
        staging_buffer_info.pQueueFamilyIndices = queue_family_indices;
    }

    auto staging_buffer = GET_OR_FORWARD(init.memory_context->create_staging_buffer(staging_buffer_info));

    VkImageCreateInfo image_info = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = format,
        .extent = {.width = init.width, .height = init.height, .depth = 1},
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = nullptr,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    };

    auto image = GET_OR_FORWARD(init.memory_context->create_gpu_image(image_info));

    VkImageViewCreateInfo image_view_info = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = image,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = format,
        .subresourceRange =
            {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
    };

    VkImageView image_view;
    if (vkCreateImageView(init.device, &image_view_info, nullptr, &image_view) != VK_SUCCESS)
        return result::err("failed to create image view!");

    return result::ok(new vulkan_image(init, staging_buffer, image, image_view));
}

void vulkan_image::write(const void* data, uint32_t size) {
    // Map and copy to staging buffer
    void* mapped_data = _memory_context->map_buffer(_staging_buffer);
    memcpy(mapped_data, data, size);
    _memory_context->unmap_buffer(_staging_buffer);

    // Create transfer command buffer
    VkCommandBufferAllocateInfo alloc_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = _transfer_command_pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };

    VkCommandBuffer command_buffer;
    if (vkAllocateCommandBuffers(_device, &alloc_info, &command_buffer) != VK_SUCCESS)
        throw std::runtime_error("Failed to allocate command buffer");

    // Begin transfer command buffer
    VkCommandBufferBeginInfo begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };

    if (vkBeginCommandBuffer(command_buffer, &begin_info) != VK_SUCCESS)
        throw std::runtime_error("Failed to begin command buffer");

    // Transition image to transfer destination
    VkImageMemoryBarrier barrier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .srcAccessMask = 0,
        .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
        .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = _image,
        .subresourceRange =
            {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
    };
    vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0,
                         nullptr, 0, nullptr, 1, &barrier);

    // Copy buffer to image
    VkBufferImageCopy region = {
        .bufferOffset = 0,
        .bufferRowLength = 0,
        .bufferImageHeight = 0,
        .imageSubresource =
            {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .mipLevel = 0,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
        .imageOffset = {.x = 0, .y = 0, .z = 0},
        .imageExtent = {.width = width(), .height = height(), .depth = 1},
    };
    vkCmdCopyBufferToImage(command_buffer, _staging_buffer, _image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    // Transition image to shader read
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0,
                         nullptr, 0, nullptr, 1, &barrier);

    // Submit transfer command buffer
    if (vkEndCommandBuffer(command_buffer) != VK_SUCCESS) throw std::runtime_error("Failed to end command buffer");

    VkSubmitInfo submit_info = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &command_buffer,
    };

    if (vkQueueSubmit(_transfer_queue, 1, &submit_info, VK_NULL_HANDLE) != VK_SUCCESS)
        throw std::runtime_error("Failed to submit command buffer");

    vkQueueWaitIdle(_transfer_queue);
    vkFreeCommandBuffers(_device, _transfer_command_pool, 1, &command_buffer);
}

VkImage vulkan_image::image() const {
    return _image;
}

VkImageView vulkan_image::image_view() const {
    return _image_view;
}

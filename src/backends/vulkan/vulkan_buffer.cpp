#include "vulkan_buffer.h"

vulkan_buffer::vulkan_buffer(const vulkan_buffer_init& init, VkBuffer staging_buffer, VkBuffer buffer)
    : graphics_buffer(init.usage, init.size),
      _device(init.device),
      _memory_context(init.memory_context),
      _staging_buffer(staging_buffer),
      _buffer(buffer),
      _transfer_command_pool(init.transfer_command_pool),
      _transfer_queue(init.transfer_queue) { }

vulkan_buffer::~vulkan_buffer() {
    _memory_context.destroy_buffer(_staging_buffer);
    _memory_context.destroy_buffer(_buffer);
}

result::ptr<graphics_buffer> vulkan_buffer::create(const vulkan_buffer_init& init) {
    auto size = init.size;

    VkBufferCreateInfo staging_buffer_info = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = nullptr,
    };

    if (init.def.transfer_family != init.def.graphics_family) {
        uint32_t queue_family_indices[] = {init.def.transfer_family.value(), init.def.graphics_family.value()};
        staging_buffer_info.sharingMode = VK_SHARING_MODE_CONCURRENT;
        staging_buffer_info.queueFamilyIndexCount = 2;
        staging_buffer_info.pQueueFamilyIndices = queue_family_indices;
    }

    auto staging_buffer = GET_OR_FORWARD(init.memory_context.create_staging_buffer(staging_buffer_info));

    VkBufferUsageFlags flags = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    if (init.usage & (int) buffer_usage::vertex) flags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    if (init.usage & (int) buffer_usage::index) flags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

    VkBufferCreateInfo buffer_info = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .usage = flags,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };

    auto buffer = GET_OR_FORWARD(init.memory_context.create_gpu_buffer(buffer_info));
    return result::ok(new vulkan_buffer(init, staging_buffer, buffer));
}

VkBuffer vulkan_buffer::buffer() const {
    return _buffer;
}

void vulkan_buffer::write(const void* data, uint32_t size) {
    // Map and copy to staging buffer
    void* mapped_data = _memory_context.map_buffer(_staging_buffer);
    memcpy(mapped_data, data, size);
    _memory_context.unmap_buffer(_staging_buffer);

    // Create transfer command buffer
    VkCommandBufferAllocateInfo alloc_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = _transfer_command_pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };

    VkCommandBuffer command_buffer;
    if (vkAllocateCommandBuffers(_device, &alloc_info, &command_buffer) != VK_SUCCESS)
        throw std::runtime_error("Failed to allocate transfer command buffer");

    // Begin command buffer
    VkCommandBufferBeginInfo begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };

    if (vkBeginCommandBuffer(command_buffer, &begin_info) != VK_SUCCESS)
        throw std::runtime_error("Failed to begin transfer command buffer");

    // Copy staging buffer to GPU buffer
    VkBufferCopy copy_region = {.size = size};
    vkCmdCopyBuffer(command_buffer, _staging_buffer, _buffer, 1, &copy_region);

    // Submit transfer command buffer
    if (vkEndCommandBuffer(command_buffer) != VK_SUCCESS)
        throw std::runtime_error("Failed to end transfer command buffer");

    VkSubmitInfo submit_info = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &command_buffer,
    };

    if (vkQueueSubmit(_transfer_queue, 1, &submit_info, VK_NULL_HANDLE) != VK_SUCCESS)
        throw std::runtime_error("Failed to submit transfer command buffer");

    vkQueueWaitIdle(_transfer_queue);

    vkFreeCommandBuffers(_device, _transfer_command_pool, 1, &command_buffer);
}

#include "vulkan_uniform_buffer.h"
vulkan_uniform_buffer::vulkan_uniform_buffer(const shader_variable_type& type, uint32_t size, VkDevice device,
                                             vulkan_sync_context* sync_context, vulkan_memory_context* memory_context,
                                             const std::vector<vulkan_uniform_buffer_buffer>& buffers)
    : graphics_uniform_buffer(type, size),
      _device(device),
      _sync_context(sync_context),
      _memory_context(memory_context),
      _buffers(buffers) { }

vulkan_uniform_buffer::~vulkan_uniform_buffer() {
    for (auto buffer : _buffers) {
        _memory_context->destroy_buffer(buffer.buffer);
    }
}

void vulkan_uniform_buffer::set_data(const shader_variable_type& type, uint32_t offset, uint32_t size,
                                     const void* data) {
    auto mapped_data = (uint8_t*) _buffers[_sync_context->current_frame()].mapped_data;
    memcpy(mapped_data + offset, data, size);
}

result::ptr<graphics_uniform_buffer> vulkan_uniform_buffer::create(const shader_variable_type& type, VkDevice device,
                                                                   vulkan_sync_context& sync_context,
                                                                   vulkan_memory_context& memory_context) {
    std::vector<vulkan_uniform_buffer_buffer> buffers;

    VkDeviceSize buffer_size = type.size;
    VkBufferCreateInfo buffer_info = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = buffer_size,
        .usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };

    for (int i = 0; i < sync_context.frames_in_flight(); i++) {
        auto buffer = GET_OR_FORWARD(memory_context.create_mapped_buffer(buffer_info));
        auto mapped_data = memory_context.map_buffer(buffer);
        buffers.push_back({buffer, mapped_data});
    }

    return result::ok(new vulkan_uniform_buffer(type, buffer_size, device, &sync_context, &memory_context, buffers));
}

VkBuffer vulkan_uniform_buffer::buffer(uint32_t frame) const {
    return _buffers[frame].buffer;
}

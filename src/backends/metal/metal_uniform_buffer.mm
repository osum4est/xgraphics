#include "metal_uniform_buffer.h"

metal_uniform_buffer::metal_uniform_buffer(const shader_variable_type& type, uint32_t size,
                                           metal_sync_context* sync_context, const std::vector<id<MTLBuffer>>& buffers)
    : graphics_uniform_buffer(type, size), _sync_context(sync_context), _buffers(buffers) { }

void metal_uniform_buffer::set_data(const shader_variable_type& type, uint32_t offset, uint32_t size,
                                    const void* data) {
    auto buffer = _buffers[_sync_context->current_frame()];
    memcpy((uint8_t*) buffer.contents + offset, data, size);
}

result::ptr<graphics_uniform_buffer>
metal_uniform_buffer::create(const shader_variable_type& type, id<MTLDevice> device, metal_sync_context& sync_context) {
    std::vector<id<MTLBuffer>> buffers;
    auto size = type.size;

    for (auto i = 0; i < sync_context.frames_in_flight(); i++) {
        auto buffer = [device newBufferWithLength:size options:MTLResourceStorageModeShared];
        buffers.push_back(buffer);
    }

    return result::ok(new metal_uniform_buffer(type, size, &sync_context, buffers));
}

id<MTLBuffer> metal_uniform_buffer::buffer() const {
    return _buffers[_sync_context->current_frame()];
}

id<MTLBuffer> metal_uniform_buffer::buffer(uint32_t frame) const {
    return _buffers[frame];
}

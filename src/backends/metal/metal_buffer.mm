#include "metal_buffer.h"

metal_buffer::metal_buffer(buffer_usage_flags usage, uint32_t size, id<MTLBuffer> buffer)
    : graphics_buffer(usage, size), _buffer(buffer) { }

result::ptr<graphics_buffer> metal_buffer::create(buffer_usage_flags usage, uint32_t size, id<MTLDevice> device) {
    id<MTLBuffer> buffer = [device newBufferWithLength:size options:MTLResourceStorageModeShared];
    return result::ok(new metal_buffer(usage, size, buffer));
}

id<MTLBuffer> metal_buffer::buffer() const {
    return _buffer;
}

void metal_buffer::write(const void* data, uint32_t size) {
    memcpy(_buffer.contents, data, size);
}

#include "xgraphics/interfaces/graphics_buffer.h"

graphics_buffer::graphics_buffer(buffer_usage_flags usage, uint32_t size) : _usage(usage), _size(size) { }

buffer_usage_flags graphics_buffer::usage() const {
    return _usage;
}

uint32_t graphics_buffer::size() const {
    return _size;
}

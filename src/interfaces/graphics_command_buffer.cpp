#include "xgraphics/interfaces/graphics_command_buffer.h"

void graphics_command_buffer::draw(uint32_t vertex_start, uint32_t vertex_count) {
    draw(vertex_start, vertex_count, 0, 1);
}

void graphics_command_buffer::draw_indexed(const graphics_buffer& index_buffer, uint32_t index_offset, index_type type,
                                           uint32_t index_start, uint32_t index_count, uint32_t vertex_offset) {
    draw_indexed(index_buffer, index_offset, type, index_start, index_count, vertex_offset, 0, 1);
}

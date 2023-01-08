#ifndef WPEX_GRAPHICS_COMMAND_BUFFER_H
#define WPEX_GRAPHICS_COMMAND_BUFFER_H

#include "graphics_buffer.h"
#include "graphics_pipeline.h"
#include "graphics_render_pass.h"
#include "graphics_resource_set.h"

enum class index_type {
    uint_16,
    uint_32,
};

class graphics_command_buffer {
  public:
    explicit graphics_command_buffer() = default;
    graphics_command_buffer(const graphics_command_buffer&) = delete;
    virtual ~graphics_command_buffer() = default;

    virtual void begin() = 0;
    virtual void end() = 0;

    virtual void begin_render_pass(const graphics_render_pass& render_pass) = 0;
    virtual void end_render_pass() = 0;

    virtual void bind_pipeline(const graphics_pipeline& pipeline) = 0;
    virtual void bind_vertex_buffer(const graphics_buffer& buffer, uint32_t offset, int index) = 0;
    virtual void bind_resource_set(const graphics_resource_set& resource_set) = 0;

    void draw(uint32_t vertex_start, uint32_t vertex_count);
    virtual void draw(uint32_t vertex_start, uint32_t vertex_count, uint32_t instance_start,
                      uint32_t instance_count) = 0;
    void draw_indexed(const graphics_buffer& index_buffer, uint32_t index_offset, index_type type, uint32_t index_start,
                      uint32_t index_count, uint32_t vertex_offset);
    virtual void draw_indexed(const graphics_buffer& index_buffer, uint32_t index_offset, index_type type,
                              uint32_t index_start, uint32_t index_count, uint32_t vertex_offset,
                              uint32_t instance_start, uint32_t instance_count) = 0;
};

#endif

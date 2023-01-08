#ifndef XGRAPHICS_METAL_COMMAND_BUFFER_H
#define XGRAPHICS_METAL_COMMAND_BUFFER_H

#import "metal_pipeline.h"
#import <Metal/Metal.h>
#import <result/result.h>
#import <xgraphics/interfaces/graphics_buffer.h>
#import <xgraphics/interfaces/graphics_command_buffer.h>

class metal_command_buffer : public graphics_command_buffer {
    id<MTLCommandQueue> _command_queue;
    id<MTLCommandBuffer> _command_buffer = nullptr;
    id<MTLRenderCommandEncoder> _render_command_encoder = nullptr;
    const metal_pipeline* _current_pipeline = nullptr;

    explicit metal_command_buffer(id<MTLCommandQueue> command_queue);

  public:
    static result::ptr<graphics_command_buffer> create(id<MTLCommandQueue> command_queue);

    [[nodiscard]] id<MTLCommandBuffer> command_buffer() const;

    void begin() override;
    void end() override;
    void begin_render_pass(const graphics_render_pass& render_pass) override;
    void end_render_pass() override;
    void bind_pipeline(const graphics_pipeline& pipeline) override;
    void bind_vertex_buffer(const graphics_buffer& buffer, uint32_t offset, int index) override;
    void bind_resource_set(const graphics_resource_set& resource_set) override;
    void draw(uint32_t vertex_start, uint32_t vertex_count, uint32_t instance_start, uint32_t instance_count) override;
    void draw_indexed(const graphics_buffer& index_buffer, uint32_t index_offset, index_type type, uint32_t index_start,
                      uint32_t index_count, uint32_t vertex_offset, uint32_t instance_start,
                      uint32_t instance_count) override;
};

#endif

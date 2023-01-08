#ifndef XGRAPHICS_METAL_PIPELINE_H
#define XGRAPHICS_METAL_PIPELINE_H

#import <Metal/Metal.h>
#import <result/result.h>
#import <xgraphics/interfaces/graphics_pipeline.h>

class metal_pipeline : public graphics_pipeline {
    id<MTLRenderPipelineState> _pipeline;
    id<MTLDepthStencilState> _depth_stencil_state;
    uint32_t _vertex_buffer_index_offset;

  public:
    explicit metal_pipeline(const graphics_pipeline_init& init, id<MTLRenderPipelineState> pipeline,
                            id<MTLDepthStencilState> depth_stencil_state, uint32_t vertex_buffer_index_offset);

    static result::ptr<graphics_pipeline> create(const graphics_pipeline_init& init, id<MTLDevice> device);

    [[nodiscard]] id<MTLRenderPipelineState> pipeline() const;
    [[nodiscard]] id<MTLDepthStencilState> depth_stencil_state() const;
    [[nodiscard]] uint32_t vertex_buffer_index_offset() const;
};

#endif

#include "metal_command_buffer.h"
#import "metal_buffer.h"
#import "metal_render_pass.h"
#import "metal_resource_set.h"

metal_command_buffer::metal_command_buffer(id<MTLCommandQueue> command_queue) : _command_queue(command_queue) { }

result::ptr<graphics_command_buffer> metal_command_buffer::create(id<MTLCommandQueue> command_queue) {
    return result::ok(new metal_command_buffer(command_queue));
}

id<MTLCommandBuffer> metal_command_buffer::command_buffer() const {
    return _command_buffer;
}

void metal_command_buffer::begin() {
    _command_buffer = [_command_queue commandBuffer];
}

void metal_command_buffer::end() {
    _current_pipeline = nullptr;
}

void metal_command_buffer::begin_render_pass(const graphics_render_pass& render_pass) {
    const auto& native_render_pass = (const metal_render_pass&) render_pass;
    const auto& native_swapchain = (const metal_swapchain&) render_pass.swapchain();

    auto drawable = native_swapchain.current_drawable();
    auto descriptor = native_render_pass.render_pass_descriptor();
    descriptor.colorAttachments[0].texture = drawable.texture;
    descriptor.depthAttachment.texture = native_swapchain.depth_stencil_texture();

    _render_command_encoder = [_command_buffer renderCommandEncoderWithDescriptor:descriptor];
}

void metal_command_buffer::end_render_pass() {
    [_render_command_encoder endEncoding];
    [_render_command_encoder release];
    _render_command_encoder = nullptr;
}

void metal_command_buffer::bind_pipeline(const graphics_pipeline& pipeline) {
    const auto& native_pipeline = (const metal_pipeline&) pipeline;
    [_render_command_encoder setRenderPipelineState:native_pipeline.pipeline()];
    [_render_command_encoder setDepthStencilState:native_pipeline.depth_stencil_state()];
    [_render_command_encoder setCullMode:MTLCullModeNone];
    _current_pipeline = &native_pipeline;
}

void metal_command_buffer::bind_vertex_buffer(const graphics_buffer& buffer, uint32_t offset, int index) {
    const auto& native_buffer = (const metal_buffer&) buffer;
    auto buffer_index = index + _current_pipeline->vertex_buffer_index_offset();
    [_render_command_encoder setVertexBuffer:native_buffer.buffer() offset:offset atIndex:buffer_index];
}

void metal_command_buffer::bind_resource_set(const graphics_resource_set& resource_set) {
    const auto& native_resource_set = (const metal_resource_set&) resource_set;

    auto vertex_buffer = native_resource_set.vertex_argument_buffer();
    if (vertex_buffer) {
        [_render_command_encoder setVertexBuffer:vertex_buffer offset:0 atIndex:resource_set.ref()->backend_number];
        const auto& resources = native_resource_set.bound_vertex_resources();
        [_render_command_encoder useResources:resources.data()
                                        count:resources.size()
                                        usage:MTLResourceUsageRead
                                       stages:MTLRenderStageVertex];
    }

    auto fragment_buffer = native_resource_set.fragment_argument_buffer();
    if (fragment_buffer) {
        [_render_command_encoder setFragmentBuffer:fragment_buffer offset:0 atIndex:resource_set.ref()->backend_number];
        const auto& resources = native_resource_set.bound_fragment_resources();
        [_render_command_encoder useResources:resources.data()
                                        count:resources.size()
                                        usage:MTLResourceUsageRead
                                       stages:MTLRenderStageFragment];
    }
}

void metal_command_buffer::draw(uint32_t vertex_start, uint32_t vertex_count, uint32_t instance_start,
                                uint32_t instance_count) {
    [_render_command_encoder drawPrimitives:MTLPrimitiveTypeTriangle
                                vertexStart:vertex_start
                                vertexCount:vertex_count
                              instanceCount:instance_count
                               baseInstance:instance_start];
}

void metal_command_buffer::draw_indexed(const graphics_buffer& index_buffer, uint32_t index_offset, index_type type,
                                        uint32_t index_start, uint32_t index_count, uint32_t vertex_offset,
                                        uint32_t instance_start, uint32_t instance_count) {
    const auto& native_buffer = (const metal_buffer&) index_buffer;
    MTLIndexType index_type;
    int type_size;

    switch (type) {
        case index_type::uint_16:
            index_type = MTLIndexTypeUInt16;
            type_size = 2;
            break;
        case index_type::uint_32:
            index_type = MTLIndexTypeUInt32;
            type_size = 4;
            break;
        default:
            throw std::runtime_error("Unsupported index type");
    }

    uint32_t final_index_offset = index_offset + index_start * type_size;
    [_render_command_encoder drawIndexedPrimitives:MTLPrimitiveTypeTriangle
                                        indexCount:index_count
                                         indexType:index_type
                                       indexBuffer:native_buffer.buffer()
                                 indexBufferOffset:final_index_offset
                                     instanceCount:instance_count
                                        baseVertex:vertex_offset
                                      baseInstance:instance_start];
}

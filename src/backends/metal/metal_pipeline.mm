#import "metal_pipeline.h"

#import "metal_shader.h"

metal_pipeline::metal_pipeline(const graphics_pipeline_init& init, id<MTLRenderPipelineState> pipeline,
                               id<MTLDepthStencilState> depth_stencil_state, uint32_t vertex_buffer_index_offset)
    : graphics_pipeline(init),
      _pipeline(pipeline),
      _depth_stencil_state(depth_stencil_state),
      _vertex_buffer_index_offset(vertex_buffer_index_offset) { }

result::ptr<graphics_pipeline> metal_pipeline::create(const graphics_pipeline_init& init, id<MTLDevice> device) {
    const auto* vertex_shader = (const metal_shader*) GET_OR_FORWARD(init.layout.vertex_shader());
    const auto* fragment_shader = (const metal_shader*) GET_OR_FORWARD(init.layout.fragment_shader());

    auto vertex_buffer_index_offset = vertex_shader->resources().resource_sets.size();

    MTLVertexDescriptor* vertex_desc = [[MTLVertexDescriptor alloc] init];
    for (int i_binding = 0; i_binding < init.vertex_bindings.size(); i_binding++) {
        const auto& binding = init.vertex_bindings[i_binding];
        auto buffer_index = i_binding + vertex_buffer_index_offset;
        for (const auto& attribute : binding.attributes) {
            MTLVertexFormat format;
            switch (attribute.format) {
                case attribute_format::float_32:
                    format = MTLVertexFormatFloat;
                    break;
                case attribute_format::int_32:
                    format = MTLVertexFormatInt;
                    break;
                case attribute_format::uint_32:
                    format = MTLVertexFormatUInt;
                    break;
                default:
                    return result::err("Unsupported attribute format");
            }

            MTLVertexAttributeDescriptor* attribute_desc = vertex_desc.attributes[attribute.ref->backend_location];
            attribute_desc.format = (MTLVertexFormat) (format + (int) attribute.dimension - 1);
            attribute_desc.offset = attribute.offset;
            attribute_desc.bufferIndex = buffer_index;
        }

        MTLVertexBufferLayoutDescriptor* binding_desc = vertex_desc.layouts[buffer_index];
        binding_desc.stride = binding.stride;
    }

    // TODO: Clean up pipeline descriptor?
    MTLRenderPipelineDescriptor* pipeline_desc = [[MTLRenderPipelineDescriptor alloc] init];
    pipeline_desc.vertexFunction = vertex_shader->function();
    pipeline_desc.fragmentFunction = fragment_shader->function();
    pipeline_desc.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm_sRGB;
    pipeline_desc.depthAttachmentPixelFormat = MTLPixelFormatDepth32Float;
    pipeline_desc.vertexDescriptor = vertex_desc;

    NSError* error = nil;
    id<MTLRenderPipelineState> pipeline = [device newRenderPipelineStateWithDescriptor:pipeline_desc error:&error];
    if (!pipeline)
        return result::err(std::string("Failed to create pipeline: ") + error.localizedDescription.UTF8String);

    // Create depth stencil state
    MTLDepthStencilDescriptor* depth_stencil_desc = [[MTLDepthStencilDescriptor alloc] init];
    depth_stencil_desc.depthCompareFunction = MTLCompareFunctionLess;
    depth_stencil_desc.depthWriteEnabled = YES;
    id<MTLDepthStencilState> depth_stencil_state = [device newDepthStencilStateWithDescriptor:depth_stencil_desc];

    return result::ok(new metal_pipeline(init, pipeline, depth_stencil_state, vertex_buffer_index_offset));
}

id<MTLRenderPipelineState> metal_pipeline::pipeline() const {
    return _pipeline;
}

id<MTLDepthStencilState> metal_pipeline::depth_stencil_state() const {
    return _depth_stencil_state;
}

uint32_t metal_pipeline::vertex_buffer_index_offset() const {
    return _vertex_buffer_index_offset;
}

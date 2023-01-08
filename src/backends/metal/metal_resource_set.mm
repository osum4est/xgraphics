#import "metal_resource_set.h"

#import "metal_shader.h"

metal_resource_set::metal_resource_set(const metal_resource_layout& layout, resource_set_ref ref,
                                       const stage_info& vertex_info, const stage_info& fragment_info,
                                       const metal_sync_context& sync_context)
    : graphics_resource_set(layout, ref),
      _vertex_info(vertex_info),
      _fragment_info(fragment_info),
      _sync_context(&sync_context) { }

result::ptr<graphics_resource_set> metal_resource_set::create(const metal_resource_layout& layout, resource_set_ref ref,
                                                              id<MTLDevice> device,
                                                              const metal_sync_context& sync_context) {
    stage_info vertex_info = {};
    stage_info fragment_info = {};

    for (const auto& resource : ref->resources) {
        for (const auto* stage : resource.stages) {
            const auto* native_stage = (const metal_shader*) stage;
            switch (stage->info().kind) {
                case shader_kind::vertex:
                    vertex_info.has_arguments = true;
                    vertex_info.function = native_stage->function();
                    vertex_info.bindings[resource.source_binding] = {};
                    break;
                case shader_kind::fragment:
                    fragment_info.has_arguments = true;
                    fragment_info.function = native_stage->function();
                    fragment_info.bindings[resource.source_binding] = {};
                    break;
            }
        }
    }

    auto init_stage = [&](stage_info& info) {
        if (!info.has_arguments) return;

        for (int i = 0; i < sync_context.frames_in_flight(); i++) {
            auto argument_encoder = [info.function newArgumentEncoderWithBufferIndex:ref->backend_number];
            auto argument_buffer = [device newBufferWithLength:argument_encoder.encodedLength
                                                       options:MTLResourceStorageModeManaged];
            [argument_encoder setArgumentBuffer:argument_buffer offset:0];
            info.arguments.push_back({argument_encoder, argument_buffer});
        }
    };

    init_stage(vertex_info);
    init_stage(fragment_info);

    return result::ok(new metal_resource_set(layout, ref, vertex_info, fragment_info, sync_context));
}

void metal_resource_set::bind_uniform_buffer(resource_binding_ref binding, const graphics_uniform_buffer& buffer) {
    const auto& native_buffer = (const metal_uniform_buffer&) buffer;

    bind_uniform_buffer(binding, native_buffer, _vertex_info);
    bind_uniform_buffer(binding, native_buffer, _fragment_info);
}

void metal_resource_set::bind_uniform_buffer(resource_binding_ref binding, const metal_uniform_buffer& buffer,
                                             metal_resource_set::stage_info& info) {
    if (!info.bindings.contains(binding->source_binding)) return;

    std::vector<id<MTLResource>> resources;
    for (int i = 0; i < _sync_context->frames_in_flight(); i++) {
        auto& argument = info.arguments[i];
        [argument.encoder setBuffer:buffer.buffer(i) offset:0 atIndex:binding->backend_binding];
        [argument.buffer didModifyRange:NSMakeRange(0, argument.encoder.encodedLength)];
        resources.push_back(buffer.buffer(i));
    }

    bind_resources(binding, resources, info);
}

void metal_resource_set::bind_sampled_image(resource_binding_ref binding, const graphics_image& image,
                                            const graphics_sampler& sampler) {
    const auto& native_image = (const metal_image&) image;
    const auto& native_sampler = (const metal_sampler&) sampler;

    bind_sampled_image(binding, native_image, native_sampler, _vertex_info);
    bind_sampled_image(binding, native_image, native_sampler, _fragment_info);
}

void metal_resource_set::bind_sampled_image(resource_binding_ref binding, const metal_image& image,
                                            const metal_sampler& sampler, metal_resource_set::stage_info& info) {
    if (!info.bindings.contains(binding->source_binding)) return;

    for (int i = 0; i < _sync_context->frames_in_flight(); i++) {
        auto& argument = info.arguments[i];
        [argument.encoder setTexture:image.texture() atIndex:binding->backend_binding];
        // TODO: Is sampler always texture binding + 1?
        [argument.encoder setSamplerState:sampler.sampler() atIndex:(binding->backend_binding + 1)];
        [argument.buffer didModifyRange:NSMakeRange(0, argument.encoder.encodedLength)];
    }

    bind_resources(binding, {image.texture()}, info);
}

void metal_resource_set::bind_resources(resource_binding_ref binding, const std::vector<id<MTLResource>>& resources,
                                        metal_resource_set::stage_info& info) {
    // Update bound buffer
    info.bindings[binding->source_binding] = resources;

    // Rebuild bound resources lists
    info.bound_resources.clear();
    for (int i = 0; i < _sync_context->frames_in_flight(); i++) {
        std::vector<id<MTLResource>> frame_resources;
        for (const auto& [binding, resources] : info.bindings) {
            if (resources.empty()) continue;
            else if (resources.size() == 1) frame_resources.push_back(resources[0]);
            else frame_resources.push_back(resources[i]);
        }
        info.bound_resources.push_back(frame_resources);
    }
}

id<MTLBuffer> metal_resource_set::vertex_argument_buffer() const {
    if (!_vertex_info.has_arguments) return nullptr;
    return _vertex_info.arguments[_sync_context->current_frame()].buffer;
}

id<MTLBuffer> metal_resource_set::fragment_argument_buffer() const {
    if (!_fragment_info.has_arguments) return nullptr;
    return _fragment_info.arguments[_sync_context->current_frame()].buffer;
}

const std::vector<id<MTLResource>>& metal_resource_set::bound_vertex_resources() const {
    return _vertex_info.bound_resources[_sync_context->current_frame()];
}

const std::vector<id<MTLResource>>& metal_resource_set::bound_fragment_resources() const {
    return _fragment_info.bound_resources[_sync_context->current_frame()];
}
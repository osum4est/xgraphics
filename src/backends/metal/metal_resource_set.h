#ifndef XGRAPHICS_METAL_RESOURCE_SET_H
#define XGRAPHICS_METAL_RESOURCE_SET_H

#import "metal_image.h"
#import "metal_resource_layout.h"
#import "metal_sampler.h"
#import "metal_sync_context.h"
#import "metal_uniform_buffer.h"
#import <Metal/Metal.h>
#import <vector>
#import <xgraphics/interfaces/graphics_resource_set.h>

class metal_resource_set : public graphics_resource_set {
    struct argument_info {
        id<MTLArgumentEncoder> encoder;
        id<MTLBuffer> buffer;
    };

    struct stage_info {
        bool has_arguments;
        id<MTLFunction> function;
        std::vector<argument_info> arguments;
        std::unordered_map<uint32_t, std::vector<id<MTLResource>>> bindings;
        std::vector<std::vector<id<MTLResource>>> bound_resources;
    };

    stage_info _vertex_info;
    stage_info _fragment_info;

    const metal_sync_context* _sync_context;

    metal_resource_set(const metal_resource_layout& layout, resource_set_ref ref, const stage_info& vertex_info,
                       const stage_info& fragment_info, const metal_sync_context& sync_context);

  public:
    static result::ptr<graphics_resource_set> create(const metal_resource_layout& layout, resource_set_ref ref,
                                                     id<MTLDevice> device, const metal_sync_context& sync_context);

    // TODO: Make order of functions, override functions, and getters consistent
    void bind_uniform_buffer(resource_binding_ref binding, const graphics_uniform_buffer& buffer) override;
    void bind_sampled_image(resource_binding_ref binding, const graphics_image& image,
                            const graphics_sampler& sampler) override;

    [[nodiscard]] id<MTLBuffer> vertex_argument_buffer() const;
    [[nodiscard]] id<MTLBuffer> fragment_argument_buffer() const;
    [[nodiscard]] const std::vector<id<MTLResource>>& bound_vertex_resources() const;
    [[nodiscard]] const std::vector<id<MTLResource>>& bound_fragment_resources() const;

  private:
    void bind_uniform_buffer(resource_binding_ref binding, const metal_uniform_buffer& buffer, stage_info& info);
    void bind_sampled_image(resource_binding_ref binding, const metal_image& image, const metal_sampler& sampler,
                            stage_info& info);
    void bind_resources(resource_binding_ref binding, const std::vector<id<MTLResource>>& resources, stage_info& info);
};

#endif

#ifndef XGRAPHICS_METAL_UNIFORM_BUFFER_H
#define XGRAPHICS_METAL_UNIFORM_BUFFER_H

#import "metal_sync_context.h"
#import <Metal/Metal.h>
#import <xgraphics/interfaces/graphics_uniform_buffer.h>

class metal_uniform_buffer : public graphics_uniform_buffer {
    metal_sync_context* _sync_context;
    std::vector<id<MTLBuffer>> _buffers;

    metal_uniform_buffer(const shader_variable_type& type, uint32_t size, metal_sync_context* sync_context,
                         const std::vector<id<MTLBuffer>>& buffers);

  protected:
    void set_data(const shader_variable_type& type, uint32_t offset, uint32_t size, const void* data) override;

  public:
    static result::ptr<graphics_uniform_buffer> create(const shader_variable_type& type, id<MTLDevice> device,
                                                       metal_sync_context& sync_context);

    [[nodiscard]] id<MTLBuffer> buffer() const;
    [[nodiscard]] id<MTLBuffer> buffer(uint32_t frame) const;
};

#endif

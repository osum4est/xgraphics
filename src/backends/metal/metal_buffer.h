#ifndef XGRAPHICS_METAL_BUFFER_H
#define XGRAPHICS_METAL_BUFFER_H

#import <Metal/Metal.h>
#import <result/result.h>
#import <xgraphics/interfaces/graphics_buffer.h>

class metal_buffer : public graphics_buffer {
    id<MTLBuffer> _buffer;

    metal_buffer(buffer_usage_flags usage, uint32_t size, id<MTLBuffer> buffer);

  public:
    static result::ptr<graphics_buffer> create(buffer_usage_flags usage, uint32_t size, id<MTLDevice> device);

    [[nodiscard]] id<MTLBuffer> buffer() const;

    void write(const void* data, uint32_t size) override;
};

#endif

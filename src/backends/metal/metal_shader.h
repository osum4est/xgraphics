#ifndef XGRAPHICS_METAL_SHADER_H
#define XGRAPHICS_METAL_SHADER_H

#import <Metal/Metal.h>
#import <result/result.h>
#import <xgraphics/interfaces/graphics_shader.h>

class metal_shader : public graphics_shader {
    id<MTLDevice> _device;
    id<MTLFunction> _function;

  public:
    metal_shader(std::unique_ptr<shader_binary> binary, id<MTLDevice> device, id<MTLFunction> function);

    static result::ptr<graphics_shader> create(std::unique_ptr<shader_binary> binary, id<MTLDevice> device);

    [[nodiscard]] id<MTLFunction> function() const;
};

#endif

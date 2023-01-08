#ifndef WPEX_GRAPHICS_SHADER_H
#define WPEX_GRAPHICS_SHADER_H

#include <xgraphics/shaders/shader_binary.h>

class graphics_shader {
    std::unique_ptr<shader_binary> _binary;

  protected:
    explicit graphics_shader(std::unique_ptr<shader_binary> binary);

  public:
    graphics_shader(const graphics_shader&) = delete;
    virtual ~graphics_shader() = default;

    [[nodiscard]] const shader_info& info() const;
    [[nodiscard]] const shader_resources& resources() const;
};

#endif

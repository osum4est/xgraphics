#include "xgraphics/interfaces/graphics_shader.h"

#include <variant>
#include <vector>

graphics_shader::graphics_shader(std::unique_ptr<shader_binary> binary) : _binary(std::move(binary)) { }

const shader_info& graphics_shader::info() const {
    return _binary->info();
}

const shader_resources& graphics_shader::resources() const {
    return _binary->resources();
}
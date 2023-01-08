#include "xgraphics/shaders/shader_compiler.h"

#include <vector>

result::ptr<shader_binary> shader_compiler::compile_vulkan(const intermediate_shader& intermediate) {
    auto spv = GET_OR_FORWARD(intermediate.to_spv());
    return result::ok(new shader_binary(std::move(spv), xgraphics_backend::vulkan));
}

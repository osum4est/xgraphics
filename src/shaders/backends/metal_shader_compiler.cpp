#include "xgraphics/shaders/shader_compiler.h"

#include <vector>

result::ptr<shader_binary> shader_compiler::compile_metal(const intermediate_shader& intermediate) {
    auto msl = GET_OR_FORWARD(intermediate.to_msl());
    return result::ok(new shader_binary(std::move(msl), xgraphics_backend::metal));
}

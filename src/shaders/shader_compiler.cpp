#include "xgraphics/shaders/shader_compiler.h"

result::ptr<shader_binary> shader_compiler::compile(const std::string& data, const shader_info& info,
                                                    xgraphics_backend backend) {
    return compile(std::vector<uint8_t>(data.begin(), data.end()), info, backend);
}

result::ptr<shader_binary> shader_compiler::compile(const std::vector<uint8_t>& data, const shader_info& info,
                                                    xgraphics_backend backend) {
    auto intermediate = GET_OR_FORWARD(intermediate_shader::from(data, info));
    return compile(*intermediate, backend);
}

result::ptr<shader_binary> shader_compiler::compile(const intermediate_shader& intermediate,
                                                    xgraphics_backend backend) {
    switch (backend) {
        case xgraphics_backend::vulkan:
            return compile_vulkan(intermediate);
        case xgraphics_backend::metal:
            return compile_metal(intermediate);
        default:
            return result::err("Unsupported backend");
    }
}

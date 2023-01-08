#ifndef WPEX_SHADER_COMPILER_H
#define WPEX_SHADER_COMPILER_H

#include "intermediate_shader.h"
#include <result/result.h>
#include <xgraphics/shaders/shader_binary.h>
#include <xgraphics/xgraphics_backend.h>

class shader_compiler {
  public:
    static result::ptr<shader_binary> compile(const std::string& data, const shader_info& info,
                                              xgraphics_backend backend);
    static result::ptr<shader_binary> compile(const std::vector<uint8_t>& data, const shader_info& info,
                                              xgraphics_backend backend);
    static result::ptr<shader_binary> compile(const intermediate_shader& intermediate, xgraphics_backend backend);

  private:
    static result::ptr<shader_binary> compile_vulkan(const intermediate_shader& intermediate);
    static result::ptr<shader_binary> compile_metal(const intermediate_shader& intermediate);
};

#endif

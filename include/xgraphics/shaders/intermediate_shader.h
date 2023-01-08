#ifndef WPEX_INTERMEDIATE_SHADER_H
#define WPEX_INTERMEDIATE_SHADER_H

#include "shader_data.h"
#include <result/result.h>
#include <string>
#include <xgraphics/shaders/shader_info.h>

class intermediate_shader {
    std::vector<uint8_t> _data;
    shader_info _info;

  public:
    explicit intermediate_shader(const std::vector<uint8_t>& data, const shader_info& info);
    intermediate_shader(const intermediate_shader&) = delete;

    static result::ptr<intermediate_shader> from(const std::vector<uint8_t>& data, const shader_info& info);

    [[nodiscard]] const std::vector<uint8_t>& data() const;
    [[nodiscard]] const shader_info& info() const;

    // Source representations
    static result::ptr<intermediate_shader> from_glsl(const std::string& data, const shader_info& info);
    static result::ptr<intermediate_shader> from_spv(const std::vector<uint8_t>& data, const shader_info& info);

    // Target representations
    [[nodiscard]] result::ptr<shader_data> to_spv() const;
    [[nodiscard]] result::ptr<shader_data> to_msl() const;
};

#endif

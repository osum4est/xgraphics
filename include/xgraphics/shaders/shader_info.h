#ifndef WPEX_SHADER_INFO_H
#define WPEX_SHADER_INFO_H

#include <string>

enum class shader_kind {
    vertex,
    fragment,
};

enum class shader_representation {
    glsl,
    hlsl,
    msl,
    spv,
};

struct shader_info {
    std::string name = "UnnamedShader";
    std::string entry_point = "main";
    shader_kind kind;
    shader_representation source_rep;
};

#endif

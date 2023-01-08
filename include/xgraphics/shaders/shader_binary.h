#ifndef WPEX_SHADER_BINARY_H
#define WPEX_SHADER_BINARY_H

#include "shader_data.h"
#include "shader_info.h"
#include <memory>
#include <xgraphics/xgraphics_backend.h>

// TODO: Add ability to save/load from disk
class shader_binary {
    std::unique_ptr<shader_data> _data;
    xgraphics_backend _backend;

  public:
    shader_binary(std::unique_ptr<shader_data> data, xgraphics_backend backend);
    shader_binary(const shader_binary&) = delete;

    [[nodiscard]] const std::vector<uint8_t>& data() const;
    [[nodiscard]] const shader_info& info() const;
    [[nodiscard]] const shader_resources& resources() const;
    [[nodiscard]] xgraphics_backend backend() const;
};

#endif

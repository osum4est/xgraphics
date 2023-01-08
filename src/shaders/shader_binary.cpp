#include "xgraphics/shaders/shader_binary.h"

#include <vector>

shader_binary::shader_binary(std::unique_ptr<shader_data> data, xgraphics_backend backend)
    : _data(std::move(data)), _backend(backend) { }

const std::vector<uint8_t>& shader_binary::data() const {
    return _data->data();
}

const shader_info& shader_binary::info() const {
    return _data->info();
}

const shader_resources& shader_binary::resources() const {
    return _data->resources();
}

xgraphics_backend shader_binary::backend() const {
    return _backend;
}

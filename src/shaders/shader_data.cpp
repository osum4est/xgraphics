#include "xgraphics/shaders/shader_data.h"

shader_data::shader_data(const std::string& data, const shader_info& info, const shader_resources& resources)
    : _data(data.begin(), data.end()), _info(info), _resources(resources) { }

shader_data::shader_data(const std::vector<uint8_t>& data, const shader_info& info, const shader_resources& resources)
    : _data(data), _info(info), _resources(resources) { }

const std::vector<uint8_t>& shader_data::data() const {
    return _data;
}

const shader_info& shader_data::info() const {
    return _info;
}
const shader_resources& shader_data::resources() const {
    return _resources;
}

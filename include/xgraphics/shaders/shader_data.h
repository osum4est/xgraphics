#ifndef WPEX_SHADER_DATA_H
#define WPEX_SHADER_DATA_H

#include "shader_info.h"
#include <vector>

#define XGRAPHICS_NUMERIC_FORMATS float_32, float_64, int_8, int_16, int_32, int_64, uint_8, uint_16, uint_32, uint_64

// TODO: Put all these under a shader:: class/namespace?

enum class shader_variable_base_type {
    numeric,
    structure,
    image,
    sampler,
    sampled_image,
};

struct shader_vector_variable {
    uint32_t components;
};

struct shader_matrix_variable {
    uint32_t columns;
    uint32_t rows;
};

struct shader_numeric_variable {
    enum class type {
        scalar,
        vector,
        matrix,
    };

    enum class size { XGRAPHICS_NUMERIC_FORMATS };

    size size;
    type type;

    std::variant<shader_vector_variable, shader_matrix_variable> data;
};

struct shader_struct_member;
struct shader_struct_variable {
    std::vector<shader_struct_member> members;
};

struct shader_variable_type {
    std::string name;
    shader_variable_base_type base_type;
    std::variant<std::monostate, shader_numeric_variable, shader_struct_variable> data;
    std::vector<uint32_t> array_sizes;
    uint32_t size;
};

struct shader_struct_member {
    std::string name;
    shader_variable_type type;
    uint32_t member_offset;
    uint32_t offset;
    uint32_t stride;
};

struct shader_variable {
    uint32_t id;
    std::string name;
    uint32_t source_location;
    uint32_t backend_location;
    shader_variable_type type;
};

struct shader_uniform {
    uint32_t id;
    std::string name;
    uint32_t source_binding;
    uint32_t backend_binding;
    shader_variable_type type;
    uint32_t set;
};

struct shader_resource_set {
    uint32_t source_number;
    uint32_t backend_number;
    std::vector<shader_uniform> uniforms;
};

struct shader_resources {
    std::vector<shader_variable> inputs;
    std::vector<shader_variable> outputs;
    std::vector<shader_resource_set> resource_sets;
};

class shader_data {
    std::vector<uint8_t> _data;
    shader_info _info;
    shader_resources _resources;

  public:
    shader_data(const std::string& data, const shader_info& info, const shader_resources& resources);
    shader_data(const std::vector<uint8_t>& data, const shader_info& info, const shader_resources& resources);

    [[nodiscard]] const std::vector<uint8_t>& data() const;
    [[nodiscard]] const shader_info& info() const;
    [[nodiscard]] const shader_resources& resources() const;
};

#endif

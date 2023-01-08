#include "xgraphics/shaders/intermediate_shader.h"

#include <shaderc/shaderc.hpp>
#include <spirv_cross.hpp>
#include <spirv_msl.hpp>

result::val<shader_resources> reflect_spv(const spirv_cross::Compiler& compiler);
result::val<shader_variable> get_spv_variable(const spirv_cross::Compiler& compiler,
                                              const spirv_cross::Resource& resource);
result::val<shader_uniform> get_spv_uniform(const spirv_cross::Compiler& compiler,
                                            const spirv_cross::Resource& resource);
result::val<shader_variable_type> get_spv_variable_type(const spirv_cross::Compiler& compiler,
                                                        const spirv_cross::SPIRType& type, uint32_t offset);

intermediate_shader::intermediate_shader(const std::vector<uint8_t>& data, const shader_info& info)
    : _data(data), _info(info) { }

result::ptr<intermediate_shader> intermediate_shader::from(const std::vector<uint8_t>& data, const shader_info& info) {
    switch (info.source_rep) {
        case shader_representation::glsl:
            return intermediate_shader::from_glsl(std::string(data.begin(), data.end()), info);
        case shader_representation::spv:
            return intermediate_shader::from_spv(data, info);
        default:
            return result::err("Unsupported source shader representation");
    }
}

const std::vector<uint8_t>& intermediate_shader::data() const {
    return _data;
}

const shader_info& intermediate_shader::info() const {
    return _info;
}

result::ptr<intermediate_shader> intermediate_shader::from_glsl(const std::string& data, const shader_info& info) {
    shaderc::Compiler compiler;
    shaderc::CompileOptions options;
    // TODO: This should be exposed through an options struct
#ifdef XGRAPHICS_DEBUG
    options.SetOptimizationLevel(shaderc_optimization_level_zero);
#else
    options.SetOptimizationLevel(shaderc_optimization_level_performance);
#endif
    options.SetSourceLanguage(shaderc_source_language_glsl);
    options.SetGenerateDebugInfo();
    // TODO?
    //    options.SetAutoMapLocations();
    //    options.SetAutoBindUniforms()

    shaderc_shader_kind shaderc_kind;
    switch (info.kind) {
        case shader_kind::vertex:
            shaderc_kind = shaderc_glsl_vertex_shader;
            break;
        case shader_kind::fragment:
            shaderc_kind = shaderc_glsl_fragment_shader;
            break;
        default:
            return result::err("Unsupported shader kind");
    }

    auto result = compiler.CompileGlslToSpv(data, shaderc_kind, info.name.c_str(), info.entry_point.c_str(), options);
    if (result.GetCompilationStatus() != shaderc_compilation_status_success)
        return result::err(result.GetErrorMessage());

    auto data_ptr = (uint8_t*) result.begin();
    size_t data_size = (result.end() - result.begin()) * sizeof(uint32_t);
    std::vector<uint8_t> result_data(data_ptr, data_ptr + data_size);
    return result::ok(new intermediate_shader(result_data, info));
}

result::ptr<intermediate_shader> intermediate_shader::from_spv(const std::vector<uint8_t>& data,
                                                               const shader_info& info) {
    // TODO: Should probably check if the SPIR-V is valid
    // Don't need to do any compiling here, since we are using SPIR-V as our intermediate representation
    return result::ok(new intermediate_shader(data, info));
}

result::ptr<shader_data> intermediate_shader::to_spv() const {
    // Don't need to do any compiling here, since we are using SPIR-V as our intermediate representation

    auto data_ptr = (uint32_t*) _data.data();
    size_t data_size = _data.size() / sizeof(uint32_t);
    std::vector<uint32_t> spv(data_ptr, data_ptr + data_size);
    spirv_cross::Compiler compiler(spv);
    auto resources = GET_OR_FORWARD(reflect_spv(compiler));
    return result::ok(new shader_data(_data, _info, resources));
}

result::ptr<shader_data> intermediate_shader::to_msl() const {
    try {
        auto data_ptr = (uint32_t*) _data.data();
        size_t data_size = _data.size() / sizeof(uint32_t);
        std::vector<uint32_t> spv(data_ptr, data_ptr + data_size);
        spirv_cross::CompilerMSL compiler(spv);
        spirv_cross::CompilerMSL::Options options;
        options.argument_buffers = true;
        options.set_msl_version(2);
        compiler.set_msl_options(options);

        auto info = _info;
        spv::ExecutionModel execution_model;
        switch (info.kind) {
            case shader_kind::vertex:
                execution_model = spv::ExecutionModelVertex;
                break;
            case shader_kind::fragment:
                execution_model = spv::ExecutionModelFragment;
                break;
            default:
                return result::err("Unsupported shader kind");
        }

        const std::string& compiled = compiler.compile();
        auto resources = GET_OR_FORWARD(reflect_spv(compiler));
        for (auto& set : resources.resource_sets)
            for (auto& uniform : set.uniforms)
                uniform.backend_binding = compiler.get_automatic_msl_resource_binding(uniform.id);

        info.entry_point = compiler.get_cleansed_entry_point_name(info.entry_point, execution_model);
        return result::ok(new shader_data(compiled, info, resources));
    } catch (const spirv_cross::CompilerError& e) {
        return result::err(e.what());
    }
}

result::val<shader_resources> reflect_spv(const spirv_cross::Compiler& compiler) {
    shader_resources resources;
    std::unordered_map<uint32_t, shader_resource_set> resource_sets;
    auto get_resource_set = [&](uint32_t set) {
        auto it = resource_sets.find(set);
        if (it != resource_sets.end()) return &it->second;
        resource_sets[set] = shader_resource_set {
            .source_number = set,
            .backend_number = set,
        };
        return &resource_sets[set];
    };

    auto compiler_resources = compiler.get_shader_resources();

    for (const auto& resource : compiler_resources.stage_inputs)
        resources.inputs.push_back(GET_OR_FORWARD(get_spv_variable(compiler, resource)));
    for (const auto& resource : compiler_resources.stage_outputs)
        resources.outputs.push_back(GET_OR_FORWARD(get_spv_variable(compiler, resource)));

    for (const auto& resource : compiler_resources.uniform_buffers) {
        auto uniform = GET_OR_FORWARD(get_spv_uniform(compiler, resource));
        get_resource_set(uniform.set)->uniforms.push_back(uniform);
    }

    for (const auto& resource : compiler_resources.sampled_images) {
        auto uniform = GET_OR_FORWARD(get_spv_uniform(compiler, resource));
        get_resource_set(uniform.set)->uniforms.push_back(uniform);
    }

    // Add resource sets to resources in order
    for (auto& resource_set : resource_sets)
        resources.resource_sets.push_back(resource_set.second);

    // Sort resource sets by their number
    std::sort(
        resources.resource_sets.begin(), resources.resource_sets.end(),
        [](const shader_resource_set& a, const shader_resource_set& b) { return a.source_number < b.source_number; });

    return result::ok(resources);
}

result::val<shader_variable> get_spv_variable(const spirv_cross::Compiler& compiler,
                                              const spirv_cross::Resource& resource) {
    uint32_t location;
    if (compiler.get_decoration_bitset(resource.id).get(spv::DecorationLocation))
        location = compiler.get_decoration(resource.id, spv::DecorationLocation);
    else return result::err("No location decoration found for resource");

    auto type = GET_OR_FORWARD(get_spv_variable_type(compiler, compiler.get_type(resource.base_type_id), 0));
    return result::ok(shader_variable {
        .id = resource.id,
        .name = compiler.get_name(resource.id),
        .source_location = location,
        .backend_location = location,
        .type = type,
    });
}

result::val<shader_uniform> get_spv_uniform(const spirv_cross::Compiler& compiler,
                                            const spirv_cross::Resource& resource) {
    uint32_t binding;
    if (compiler.get_decoration_bitset(resource.id).get(spv::DecorationBinding))
        binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
    else return result::err("No binding decoration found for resource");

    uint32_t set;
    if (compiler.get_decoration_bitset(resource.id).get(spv::DecorationDescriptorSet))
        set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
    else return result::err("No set decoration found for resource");

    auto type = GET_OR_FORWARD(get_spv_variable_type(compiler, compiler.get_type(resource.base_type_id), 0));
    return result::ok(shader_uniform {
        .id = resource.id,
        .name = compiler.get_name(resource.id),
        .source_binding = binding,
        .backend_binding = binding,
        .type = type,
        .set = set,
    });
}

result::val<shader_variable_type> get_spv_variable_type(const spirv_cross::Compiler& compiler,
                                                        const spirv_cross::SPIRType& type, uint32_t offset) {
    shader_variable_type variable_type = {
        .name = compiler.get_name(type.self),
    };

    if (type.basetype == spirv_cross::SPIRType::Struct) {
        shader_struct_variable structure;

        for (int i = 0; i < type.member_types.size(); i++) {
            auto member_offset = compiler.type_struct_member_offset(type, i);
            auto member_type =
                GET_OR_FORWARD(get_spv_variable_type(compiler, compiler.get_type(type.member_types[i]), member_offset));

            auto size = (uint32_t) compiler.get_declared_struct_member_size(type, i);
            for (const auto& array_size : member_type.array_sizes)
                size /= array_size;

            structure.members.push_back(shader_struct_member {
                .name = compiler.get_member_name(type.self, i),
                .type = member_type,
                .member_offset = member_offset,
                .offset = offset + member_offset,
                .stride = size,
            });
        }

        variable_type.base_type = shader_variable_base_type::structure;
        variable_type.data = structure;
        variable_type.size = (uint32_t) compiler.get_declared_struct_size(type);
    } else if (type.basetype == spirv_cross::SPIRType::Image) {
        variable_type.base_type = shader_variable_base_type::image;
        variable_type.size = 0; // TODO
    } else if (type.basetype == spirv_cross::SPIRType::Sampler) {
        variable_type.base_type = shader_variable_base_type::sampler;
        variable_type.size = 0; // TODO
    } else if (type.basetype == spirv_cross::SPIRType::SampledImage) {
        variable_type.base_type = shader_variable_base_type::sampled_image;
        variable_type.size = 0; // TODO
    } else {
        shader_numeric_variable numeric;

        switch (type.basetype) {
            case spirv_cross::SPIRType::SByte:
                numeric.size = shader_numeric_variable::size::int_8;
                break;
            case spirv_cross::SPIRType::UByte:
                numeric.size = shader_numeric_variable::size::uint_8;
                break;
            case spirv_cross::SPIRType::Short:
                numeric.size = shader_numeric_variable::size::int_16;
                break;
            case spirv_cross::SPIRType::UShort:
                numeric.size = shader_numeric_variable::size::uint_16;
                break;
            case spirv_cross::SPIRType::Int:
                numeric.size = shader_numeric_variable::size::int_32;
                break;
            case spirv_cross::SPIRType::UInt:
                numeric.size = shader_numeric_variable::size::uint_32;
                break;
            case spirv_cross::SPIRType::Int64:
                numeric.size = shader_numeric_variable::size::int_64;
                break;
            case spirv_cross::SPIRType::UInt64:
                numeric.size = shader_numeric_variable::size::uint_64;
                break;
            case spirv_cross::SPIRType::Float:
                numeric.size = shader_numeric_variable::size::float_32;
                break;
            case spirv_cross::SPIRType::Double:
                numeric.size = shader_numeric_variable::size::float_64;
                break;
            default:
                return result::err("Unsupported resource type");
        }

        if (type.vecsize == 1 && type.columns == 1) {
            numeric.type = shader_numeric_variable::type::scalar;
        } else if (type.vecsize > 1 && type.columns == 1) {
            numeric.type = shader_numeric_variable::type::vector;
            numeric.data = shader_vector_variable {.components = type.vecsize};
        } else {
            numeric.type = shader_numeric_variable::type::matrix;
            numeric.data = shader_matrix_variable {.columns = type.columns, .rows = type.vecsize};
        }

        variable_type.base_type = shader_variable_base_type::numeric;
        variable_type.data = numeric;
        variable_type.size = type.width / 8 * type.vecsize * type.columns;
    }

    variable_type.array_sizes = std::vector<uint32_t>();
    for (auto size : type.array) {
        variable_type.array_sizes.push_back(size);
        variable_type.size *= size;
    }

    return result::ok(variable_type);
}

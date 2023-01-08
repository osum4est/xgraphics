#include "xgraphics/interfaces/graphics_resource_layout.h"

graphics_resource_layout::graphics_resource_layout(const std::vector<const graphics_shader*>& stages)
    : _stages(stages), _resources(*merge_resources(stages)) { }

std::unique_ptr<graphics_resource_layout::merged_resources>
graphics_resource_layout::merge_resources(const std::vector<const graphics_shader*>& stages) {
    auto resources = std::make_unique<merged_resources>();

    std::unordered_map<uint32_t, resource_set_ref_t> resource_sets;

    for (const auto* stage : stages) {
        const auto& stage_resources = stage->resources();

        for (const auto& attribute : stage_resources.inputs)
            resources->attributes.push_back(attribute_ref_t {attribute.source_location, attribute.backend_location});

        for (const auto& set : stage_resources.resource_sets) {
            auto& merged_set = resource_sets[set.source_number];
            merged_set.source_number = set.source_number;
            merged_set.backend_number = set.backend_number;

            for (const auto& uniform : set.uniforms) {
                resource_binding_ref_t* existing_binding = nullptr;
                for (auto& resource : merged_set.resources) {
                    if (resource.source_binding == uniform.source_binding) {
                        existing_binding = &resource;
                        break;
                    }
                }

                if (existing_binding == nullptr) {
                    merged_set.resources.push_back(resource_binding_ref_t {
                        uniform.source_binding, uniform.backend_binding, &uniform.type, {stage}});
                } else {
                    existing_binding->stages.push_back(stage);
                }
            }
        }
    }

    for (auto& set : resource_sets)
        resources->resource_sets.push_back(set.second);

    return resources;
}

std::vector<const graphics_shader*> graphics_resource_layout::stages() const {
    return _stages;
}

result::val<const graphics_shader*> graphics_resource_layout::vertex_shader() const {
    for (auto stage : _stages)
        if (stage->info().kind == shader_kind::vertex) return result::ok(stage);
    return result::err("Vertex shader not found");
}

result::val<const graphics_shader*> graphics_resource_layout::fragment_shader() const {
    for (auto stage : _stages)
        if (stage->info().kind == shader_kind::fragment) return result::ok(stage);
    return result::err("Fragment shader not found");
}

result::val<attribute_ref> graphics_resource_layout::attribute_by_name(const std::string& name) const {
    auto vertex_shader = GET_OR_FORWARD(this->vertex_shader());
    for (const auto& input : vertex_shader->resources().inputs)
        if (input.name == name) return attribute_by_location(input.source_location);
    return result::err("Attribute not found");
}

result::val<attribute_ref> graphics_resource_layout::attribute_by_location(uint32_t location) const {
    for (const auto& attribute : _resources.attributes)
        if (attribute.source_location == location) return result::ok(&attribute);
    return result::err("Attribute not found");
}

result::val<resource_set_ref> graphics_resource_layout::resource_set_by_number(uint32_t number) const {
    for (const auto& set : _resources.resource_sets)
        if (set.source_number == number) return result::ok(&set);
    return result::err("Resource set not found");
}

result::val<resource_binding_ref> graphics_resource_layout::resource_by_name(resource_set_ref set,
                                                                             const std::string& name) const {
    std::vector<const shader_uniform*> uniforms;
    for (const auto& stage : _stages)
        for (const auto& resource_set : stage->resources().resource_sets)
            if (resource_set.source_number == set->source_number)
                for (const auto& uniform : resource_set.uniforms)
                    if (uniform.name == name) uniforms.push_back(&uniform);

    if (uniforms.empty()) return result::err("Resource not found");

    auto binding = uniforms[0]->source_binding;
    for (const auto& uniform : uniforms)
        if (uniform->source_binding != binding) return result::err("Multiple resources found with that name");

    return resource_by_binding(set, binding);
}

result::val<resource_binding_ref> graphics_resource_layout::resource_by_binding(resource_set_ref set,
                                                                                uint32_t binding) const {
    for (const auto& resource : set->resources)
        if (resource.source_binding == binding) return result::ok(&resource);
    return result::err("Resource not found");
}
#ifndef WPEX_GRAPHICS_RESOURCE_LAYOUT_H
#define WPEX_GRAPHICS_RESOURCE_LAYOUT_H

#include "graphics_shader.h"
#include <result/result.h>
#include <vector>
#include <xgraphics/shaders/shader_data.h>

struct attribute_ref_t {
    uint32_t source_location;
    uint32_t backend_location;
};
typedef const attribute_ref_t* attribute_ref;

struct resource_binding_ref_t;
struct resource_set_ref_t {
    uint32_t source_number;
    uint32_t backend_number;
    std::vector<resource_binding_ref_t> resources;
};
typedef const resource_set_ref_t* resource_set_ref;

struct resource_binding_ref_t {
    uint32_t source_binding;
    uint32_t backend_binding;
    const shader_variable_type* type;
    std::vector<const graphics_shader*> stages;
};
typedef const resource_binding_ref_t* resource_binding_ref;

class graphics_resource_layout {
    std::vector<const graphics_shader*> _stages;
    struct merged_resources {
        std::vector<attribute_ref_t> attributes;
        std::vector<resource_set_ref_t> resource_sets;
    } _resources;

  protected:
    explicit graphics_resource_layout(const std::vector<const graphics_shader*>& stages);

    static std::unique_ptr<merged_resources> merge_resources(const std::vector<const graphics_shader*>& stages);

  public:
    graphics_resource_layout(const graphics_resource_layout&) = delete;
    virtual ~graphics_resource_layout() = default;

    [[nodiscard]] std::vector<const graphics_shader*> stages() const;
    [[nodiscard]] result::val<const graphics_shader*> vertex_shader() const;
    [[nodiscard]] result::val<const graphics_shader*> fragment_shader() const;

    [[nodiscard]] result::val<attribute_ref> attribute_by_name(const std::string& name) const;
    [[nodiscard]] result::val<attribute_ref> attribute_by_location(uint32_t location) const;

    [[nodiscard]] result::val<resource_set_ref> resource_set_by_number(uint32_t number) const;
    [[nodiscard]] result::val<resource_binding_ref> resource_by_name(resource_set_ref set,
                                                                     const std::string& name) const;
    [[nodiscard]] result::val<resource_binding_ref> resource_by_binding(resource_set_ref set, uint32_t binding) const;
};

#endif

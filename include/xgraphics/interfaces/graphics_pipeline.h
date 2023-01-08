#ifndef WPEX_GRAPHICS_PIPELINE_H
#define WPEX_GRAPHICS_PIPELINE_H

#include "graphics_render_pass.h"
#include "graphics_resource_layout.h"
#include "graphics_shader.h"
#include <xgraphics/shaders/shader_data.h>

enum class attribute_format { XGRAPHICS_NUMERIC_FORMATS };

enum class attribute_dimension {
    vec_1 = 1,
    vec_2 = 2,
    vec_3 = 3,
    vec_4 = 4,
};

// TODO: If we want to support structs, we may need to change this
struct attribute {
    attribute_ref ref;
    attribute_format format;
    attribute_dimension dimension;
    uint32_t offset;
};

struct vertex_binding {
    uint32_t stride;
    std::vector<attribute> attributes;
};

struct graphics_pipeline_init {
    const graphics_resource_layout& layout;
    const graphics_render_pass& render_pass;
    std::vector<vertex_binding> vertex_bindings;
};

class graphics_pipeline {
    graphics_pipeline_init _init;

  protected:
    explicit graphics_pipeline(const graphics_pipeline_init& init);

  public:
    graphics_pipeline(const graphics_pipeline&) = delete;
    virtual ~graphics_pipeline() = default;
};

#endif

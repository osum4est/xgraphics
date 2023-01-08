#ifndef WPEX_GRAPHICS_RESOURCE_SET_H
#define WPEX_GRAPHICS_RESOURCE_SET_H

#include "graphics_image.h"
#include "graphics_resource_layout.h"
#include "graphics_sampler.h"
#include "graphics_uniform_buffer.h"

class graphics_resource_set {
    const graphics_resource_layout* _layout;
    resource_set_ref _ref;

  protected:
    explicit graphics_resource_set(const graphics_resource_layout& layout, resource_set_ref ref);

  public:
    graphics_resource_set(const graphics_resource_set&) = delete;
    virtual ~graphics_resource_set() = default;

    [[nodiscard]] const graphics_resource_layout& layout() const;
    [[nodiscard]] const resource_set_ref& ref() const;

    // TODO: Need to make sure buffer type matches uniform type
    virtual void bind_uniform_buffer(resource_binding_ref binding, const graphics_uniform_buffer& buffer) = 0;
    virtual void bind_sampled_image(resource_binding_ref binding, const graphics_image& image,
                                    const graphics_sampler& sampler) = 0;
};

#endif

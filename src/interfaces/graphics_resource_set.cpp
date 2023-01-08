#include "xgraphics/interfaces/graphics_resource_set.h"

graphics_resource_set::graphics_resource_set(const graphics_resource_layout& layout, resource_set_ref ref)
    : _layout(&layout), _ref(ref) { }

const graphics_resource_layout& graphics_resource_set::layout() const {
    return *_layout;
}
const resource_set_ref& graphics_resource_set::ref() const {
    return _ref;
}

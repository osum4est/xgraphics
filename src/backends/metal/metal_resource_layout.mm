#import "metal_resource_layout.h"

metal_resource_layout::metal_resource_layout(const std::vector<const graphics_shader*>& stages)
    : graphics_resource_layout(stages) { }

result::ptr<graphics_resource_layout> metal_resource_layout::create(const std::vector<const graphics_shader*>& stages) {
    return result::ok(new metal_resource_layout(stages));
}

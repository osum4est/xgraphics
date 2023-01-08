#include "vulkan_xgraphics.h"

#include "vulkan_instance.h"

result::ptr<graphics_instance> vulkan_xgraphics::create_instance(const graphics_config& config) {
    return vulkan_instance::create(config);
}

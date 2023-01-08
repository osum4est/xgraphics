#include "xgraphics/xgraphics.h"

#ifdef XGRAPHICS_VULKAN_SUPPORTED
#include "backends/vulkan/vulkan_xgraphics.h"
#endif

#ifdef XGRAPHICS_METAL_SUPPORTED
#include "backends/metal/metal_xgraphics.h"
#endif

result::ptr<graphics_instance> xgraphics::create_instance(const graphics_config& config) {
    switch (config.backend) {
#ifdef XGRAPHICS_VULKAN_SUPPORTED
        case xgraphics_backend::vulkan:
            return vulkan_xgraphics::create_instance(config);
#endif

#ifdef XGRAPHICS_METAL_SUPPORTED
        case xgraphics_backend::metal:
            return metal_xgraphics::create_instance(config);
#endif
        default:
            return result::err("Unsupported backend");
    }
}

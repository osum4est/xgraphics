#import "metal_xgraphics.h"

#import "metal_instance.h"

result::ptr<graphics_instance> metal_xgraphics::create_instance(const graphics_config& config) {
    return metal_instance::create(config);
}

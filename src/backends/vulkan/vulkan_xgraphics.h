#ifndef XGRAPHICS_VULKAN_XGRAPHICS_H
#define XGRAPHICS_VULKAN_XGRAPHICS_H

#include <xgraphics/xgraphics.h>

class vulkan_xgraphics {
  public:
    static result::ptr<graphics_instance> create_instance(const graphics_config& config);
};

#endif

#ifndef XGRAPHICS_METAL_XGRAPHICS_H
#define XGRAPHICS_METAL_XGRAPHICS_H

#include <xgraphics/xgraphics.h>

class metal_xgraphics {
  public:
    static result::ptr<graphics_instance> create_instance(const graphics_config& config);
};

#endif

#ifndef WPEX_XGRAPHICS_H
#define WPEX_XGRAPHICS_H

#include "interfaces/graphics_instance.h"
#include <result/result.h>

class xgraphics {
  public:
    static result::ptr<graphics_instance> create_instance(const graphics_config& config);
};

#endif

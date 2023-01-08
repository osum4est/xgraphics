#ifndef XGRAPHICS_MACOS_GRAPHICS_HELPERS_H
#define XGRAPHICS_MACOS_GRAPHICS_HELPERS_H

#include <result/result.h>

class macos_graphics_helpers {
  public:
    // TODO: xgraphics should call this and pass to backends instead
    static result::val<void*> get_metal_layer(void* native_window_handle);
};

#endif
#ifndef WPEX_GRAPHICS_CONFIG_H
#define WPEX_GRAPHICS_CONFIG_H

#include "xgraphics_backend.h"

struct graphics_config {
    xgraphics_backend backend;
    void* native_window_handle;
    int frames_in_flight = 2;
};

#endif

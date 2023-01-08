#ifndef XGRAPHICS_METAL_DEVICE_DEF_H
#define XGRAPHICS_METAL_DEVICE_DEF_H

#import <Metal/Metal.h>
#import <xgraphics/xgraphics.h>

struct metal_device_def : public graphics_device_def {
    id<MTLDevice> metal_device;
};

#endif

#include "macos_graphics_helpers.h"

#import <MetalKit/MetalKit.h>
#import <objc/NSObject.h>

result::val<void*> macos_graphics_helpers::get_metal_layer(void* native_window_handle) {
    NSObject* obj = (NSObject*) native_window_handle;
    NSView* view;

    if ([obj isKindOfClass:[NSView class]]) view = (NSView*) obj;
    else if ([obj isKindOfClass:[NSWindow class]]) view = [((NSWindow*) obj) contentView];
    else return result::err(std::string("Native window handle must be a NSView or NSWindow"));

    CALayer* layer = [view layer];
    CAMetalLayer* metal_layer = nullptr;

    if (layer != nullptr && [layer isKindOfClass:[CAMetalLayer class]]) {
        metal_layer = (CAMetalLayer*) layer;
    } else {
        metal_layer = [CAMetalLayer layer];
        [view setLayer:metal_layer];
        [view setWantsLayer:YES];
    }

    // [metal_layer setDisplaySyncEnabled:NO];

    return result::ok((void*) metal_layer);
}

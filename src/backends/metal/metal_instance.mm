#import "metal_instance.h"

#import "../common/macos/macos_graphics_helpers.h"
#import "metal_device.h"
#import "metal_device_def.h"

metal_instance::metal_instance(const graphics_config& config, CAMetalLayer* layer)
    : graphics_instance(config), _layer(layer) { }

result::ptr<graphics_instance> metal_instance::create(const graphics_config& config) {
    auto layer = (CAMetalLayer*) macos_graphics_helpers::get_metal_layer(config.native_window_handle).get();
    return result::ok(new metal_instance(config, layer));
}

std::vector<std::unique_ptr<graphics_device_def>> metal_instance::list_devices() {
    auto devices = std::vector<std::unique_ptr<graphics_device_def>>();
    auto metal_devices = MTLCopyAllDevices();
    for (int i = 0; i < metal_devices.count; i++) {
        id<MTLDevice> metal_device = metal_devices[i];
        auto device = std::make_unique<metal_device_def>();
        device->name = metal_device.name.UTF8String;
        device->type = metal_device.isLowPower ? device_type::integrated : device_type::discrete;
        // TODO: How to clean up metal_device? (__bridge_transfer? __bridge?)
        device->metal_device = metal_device;
        devices.push_back(std::unique_ptr<graphics_device_def>((graphics_device_def*) device.release()));
    }

    return devices;
}

result::ptr<graphics_device> metal_instance::create_device(std::unique_ptr<graphics_device_def> def) {
    const auto& native_def = (const metal_device_def&) *def;
    _layer.device = native_def.metal_device;
    return metal_device::create(std::move(def), config(), _layer);
}

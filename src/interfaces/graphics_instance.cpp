#include "xgraphics/interfaces/graphics_instance.h"

graphics_instance::graphics_instance(const graphics_config& config) : _config(config) { }

const graphics_config& graphics_instance::config() const {
    return _config;
}

result::ptr<graphics_device> graphics_instance::create_default_device() {
    auto devices = list_devices();
    if (devices.empty()) return result::err("No suitable devices found");

    std::unique_ptr<graphics_device_def> def;

    for (auto& device : devices) {
        if (device->type == device_type::discrete) {
            def = std::move(device);
            break;
        }
    }

    if (!def) def = std::move(devices[0]);

    return create_device(std::move(def));
}

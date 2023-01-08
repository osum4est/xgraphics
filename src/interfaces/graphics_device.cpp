#include "xgraphics/interfaces/graphics_device.h"

graphics_device::graphics_device(std::unique_ptr<graphics_device_def> def, const graphics_config& config)
    : _def(std::move(def)), _config(config) { }

void graphics_device::frame_changed(int current_frame) { }

const graphics_device_def& graphics_device::def() const {
    return *_def;
}

int graphics_device::current_frame() const {
    return _current_frame;
}

void graphics_device::advance_frame() {
    wait_for_frame();
    _current_frame = (_current_frame + 1) % _config.frames_in_flight;
    frame_changed(_current_frame);
}

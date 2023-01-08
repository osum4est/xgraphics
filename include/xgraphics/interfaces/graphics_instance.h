#ifndef WPEX_GRAPHICS_INSTANCE_H
#define WPEX_GRAPHICS_INSTANCE_H

#include "graphics_device.h"
#include "graphics_device_def.h"
#include <vector>
#include <xgraphics/graphics_config.h>

class graphics_instance {
    graphics_config _config;

  public:
    explicit graphics_instance(const graphics_config& config);
    graphics_instance(const graphics_instance&) = delete;
    virtual ~graphics_instance() = default;

    [[nodiscard]] const graphics_config& config() const;

    // TODO: Have instance store graphics_device_def, and just return references
    virtual std::vector<std::unique_ptr<graphics_device_def>> list_devices() = 0;
    result::ptr<graphics_device> create_default_device();
    virtual result::ptr<graphics_device> create_device(std::unique_ptr<graphics_device_def> def) = 0;
};

#endif

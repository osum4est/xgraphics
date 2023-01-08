#ifndef XGRAPHICS_VULKAN_INSTANCE_H
#define XGRAPHICS_VULKAN_INSTANCE_H

#ifdef __APPLE__
#define VK_USE_PLATFORM_MACOS_MVK
#endif

#include <vulkan/vulkan.h>
#include <xgraphics/xgraphics.h>

class vulkan_instance : public graphics_instance {
    VkInstance _instance;
    VkSurfaceKHR _surface;

  public:
    explicit vulkan_instance(const graphics_config& config, VkInstance instance, VkSurfaceKHR surface);
    ~vulkan_instance() override;

    static result::ptr<graphics_instance> create(const graphics_config& config);

    std::vector<std::unique_ptr<graphics_device_def>> list_devices() override;
    result::ptr<graphics_device> create_device(std::unique_ptr<graphics_device_def> def) override;
};

#endif
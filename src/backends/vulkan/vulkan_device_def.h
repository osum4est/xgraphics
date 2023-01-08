#ifndef XGRAPHICS_VULKAN_DEVICE_DEF_H
#define XGRAPHICS_VULKAN_DEVICE_DEF_H

#include <optional>
#include <vector>
#include <vulkan/vulkan.h>
#include <xgraphics/xgraphics.h>

struct vulkan_device_def : public graphics_device_def {
    VkPhysicalDevice physical_device;
    std::optional<uint32_t> graphics_family;
    std::optional<uint32_t> present_family;
    std::optional<uint32_t> transfer_family;
    std::vector<const char*> required_extensions;
};

#endif

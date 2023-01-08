#include "vulkan_instance.h"

#include "../common/macos/macos_graphics_helpers.h"
#include "vulkan_device.h"

vulkan_instance::vulkan_instance(const graphics_config& config, VkInstance instance, VkSurfaceKHR surface)
    : graphics_instance(config), _instance(instance), _surface(surface) { }

vulkan_instance::~vulkan_instance() {
    vkDestroySurfaceKHR(_instance, _surface, nullptr);
    vkDestroyInstance(_instance, nullptr);
}

result::ptr<graphics_instance> vulkan_instance::create(const graphics_config& config) {
    // Create instance
    VkApplicationInfo app_info = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "xgraphics application",
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = "xgraphics",
        .engineVersion = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion = VK_API_VERSION_1_0,
    };

    std::vector<const char*> extensions = {
        VK_KHR_SURFACE_EXTENSION_NAME,
#ifdef __APPLE__
        VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME, VK_MVK_MACOS_SURFACE_EXTENSION_NAME,
        VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME
#endif
        // TODO: Add other platforms
    };

    const std::vector<const char*> enabled_layers = {"VK_LAYER_KHRONOS_validation"};

    VkInstanceCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR,
        .pApplicationInfo = &app_info,
        .enabledLayerCount = (uint32_t) enabled_layers.size(),
        .ppEnabledLayerNames = enabled_layers.data(),
        .enabledExtensionCount = (uint32_t) extensions.size(),
        .ppEnabledExtensionNames = extensions.data(),
    };

    VkInstance instance;
    VkResult result = vkCreateInstance(&create_info, nullptr, &instance);
    if (result != VK_SUCCESS) return result::err("Failed to create Vulkan instance");

    // Create surface
    VkSurfaceKHR surface;
#ifdef __APPLE__
    VkMacOSSurfaceCreateInfoMVK surface_create_info = {
        .sType = VK_STRUCTURE_TYPE_MACOS_SURFACE_CREATE_INFO_MVK,
        .pNext = nullptr,
        .flags = 0,
        .pView = macos_graphics_helpers::get_metal_layer(config.native_window_handle).get(),
    };
    if (vkCreateMacOSSurfaceMVK(instance, &surface_create_info, nullptr, &surface) != VK_SUCCESS)
        return result::err("Failed to create surface");
#endif
    // TODO: Add other platforms

    return result::ok(new vulkan_instance(config, instance, surface));
}

std::vector<std::unique_ptr<graphics_device_def>> vulkan_instance::list_devices() {
    std::vector<std::unique_ptr<graphics_device_def>> devices;

    uint32_t device_count = 0;
    vkEnumeratePhysicalDevices(_instance, &device_count, nullptr);
    std::vector<VkPhysicalDevice> physical_devices(device_count);
    vkEnumeratePhysicalDevices(_instance, &device_count, physical_devices.data());

    for (auto physical_device : physical_devices) {
        auto def = vulkan_device::create_def(physical_device, _surface);
        if (def.is_ok()) devices.push_back(std::move(def.get()));
    }

    return devices;
}

result::ptr<graphics_device> vulkan_instance::create_device(std::unique_ptr<graphics_device_def> def) {
    vulkan_device_init init = {
        .instance = _instance,
        .def = std::move(def),
        .config = config(),
        .surface = _surface,
    };

    return vulkan_device::create(init);
}

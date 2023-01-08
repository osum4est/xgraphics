#include "vulkan_device.h"

#include "vulkan_buffer.h"
#include "vulkan_command_buffer.h"
#include "vulkan_device_def.h"
#include "vulkan_image.h"
#include "vulkan_pipeline.h"
#include "vulkan_render_pass.h"
#include "vulkan_resource_layout.h"
#include "vulkan_resource_set.h"
#include "vulkan_sampler.h"
#include "vulkan_shader.h"
#include "vulkan_swapchain.h"
#include "vulkan_uniform_buffer.h"
#include <set>

const std::vector<const char*> vulkan_device::REQUIRED_EXTENSIONS = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    VK_KHR_MAINTENANCE1_EXTENSION_NAME,
};

vulkan_device::vulkan_device(vulkan_device_init& init, vulkan_device_state& state)
    : graphics_device(std::move(init.def), init.config),
      _device(state.device),
      _surface(init.surface),
      _graphics_queue(state.graphics_queue),
      _present_queue(state.present_queue),
      _transfer_queue(state.transfer_queue),
      _command_pool(state.command_pool),
      _transfer_command_pool(state.transfer_command_pool),
      _sync_context(std::move(state.sync_context)),
      _memory_context(std::move(state.memory_context)) { }

vulkan_device::~vulkan_device() {
    vkDestroyCommandPool(_device, _command_pool, nullptr);
    vkDestroyDevice(_device, nullptr);
}

void vulkan_device::wait_for_frame() {
    VkFence fence = _sync_context->gpu_wait_fence();
    vkWaitForFences(_device, 1, &fence, VK_TRUE, UINT64_MAX);
    vkResetFences(_device, 1, &fence);
}

void vulkan_device::frame_changed(int current_frame) {
    _sync_context->set_current_frame(current_frame);
}

result::ptr<graphics_device_def> vulkan_device::create_def(VkPhysicalDevice physical_device, VkSurfaceKHR surface) {
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(physical_device, &properties);

    // Create def
    auto device = std::make_unique<vulkan_device_def>();
    device->physical_device = physical_device;
    device->name = properties.deviceName;
    switch (properties.deviceType) {
        case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
            device->type = device_type::integrated;
            break;
        case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
            device->type = device_type::discrete;
            break;
        default:
            device->type = device_type::other;
            break;
    }

    // Check queue families
    uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, nullptr);
    std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_families.data());
    for (int i = 0; i < queue_families.size(); i++) {
        if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) device->graphics_family = i;
        if (queue_families[i].queueFlags & VK_QUEUE_TRANSFER_BIT &&
            !(queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT))
            device->transfer_family = i;

        VkBool32 present_support = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, surface, &present_support);
        if (present_support) device->present_family = i;
    }

    if (!device->graphics_family.has_value()) return result::err("Device must support graphics queue family");
    if (!device->present_family.has_value()) return result::err("Device must support present queue family");
    if (!device->transfer_family.has_value()) device->transfer_family = device->graphics_family;

    // Check extensions
    uint32_t extension_count = 0;
    vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extension_count, nullptr);
    std::vector<VkExtensionProperties> available_extensions(extension_count);
    vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extension_count, available_extensions.data());
    std::set<std::string> required_extensions(REQUIRED_EXTENSIONS.begin(), REQUIRED_EXTENSIONS.end());
    for (const auto& extension : available_extensions) {
        required_extensions.erase(extension.extensionName);

        // If VK_KHR_portability_subset is supported, we must enable it
        if (extension.extensionName == std::string("VK_KHR_portability_subset")) {
            device->required_extensions.push_back("VK_KHR_portability_subset");
        }
    }
    if (!required_extensions.empty()) return result::err("Device does not support required extensions");

    // Check swap chain support
    uint32_t format_count = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, nullptr);
    if (format_count == 0) return result::err("Device does not support any surface formats");

    uint32_t present_mode_count = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count, nullptr);
    if (present_mode_count == 0) return result::err("Device does not support any present modes");

    // Check anisotropy support
    VkPhysicalDeviceFeatures features;
    vkGetPhysicalDeviceFeatures(physical_device, &features);
    if (!features.samplerAnisotropy) return result::err("Device does not support anisotropic filtering");

    return result::ok(device.release());
}

result::ptr<graphics_device> vulkan_device::create(vulkan_device_init& init) {
    const auto& native_def = (const vulkan_device_def&) *init.def;
    auto physical_device = native_def.physical_device;

    std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
    std::set<uint32_t> unique_queue_families = {native_def.graphics_family.value(), native_def.present_family.value()};

    float queue_priority = 1.0f;
    for (uint32_t queue_family : unique_queue_families) {
        VkDeviceQueueCreateInfo queue_create_info = {
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = queue_family,
            .queueCount = 1,
            .pQueuePriorities = &queue_priority,
        };
        queue_create_infos.push_back(queue_create_info);
    }

    VkPhysicalDeviceFeatures device_features = {
        .samplerAnisotropy = VK_TRUE,
    };

    std::vector<const char*> extensions = {};
    for (const auto extension : REQUIRED_EXTENSIONS)
        extensions.push_back(extension);
    for (const auto& extension : native_def.required_extensions)
        extensions.push_back(extension);

    VkDeviceCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .queueCreateInfoCount = (uint32_t) queue_create_infos.size(),
        .pQueueCreateInfos = queue_create_infos.data(),
        .enabledExtensionCount = (uint32_t) extensions.size(),
        .ppEnabledExtensionNames = extensions.data(),
        .pEnabledFeatures = &device_features,
    };

    VkDevice device;
    if (vkCreateDevice(physical_device, &create_info, nullptr, &device) != VK_SUCCESS)
        return result::err("Failed to create Vulkan device");

    // Get queues
    VkQueue graphics_queue;
    VkQueue present_queue;
    VkQueue transfer_queue;
    vkGetDeviceQueue(device, native_def.graphics_family.value(), 0, &graphics_queue);
    vkGetDeviceQueue(device, native_def.present_family.value(), 0, &present_queue);
    vkGetDeviceQueue(device, native_def.transfer_family.value(), 0, &transfer_queue);

    // Create command pool
    VkCommandPoolCreateInfo pool_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = native_def.graphics_family.value(),
    };
    VkCommandPool command_pool;
    if (vkCreateCommandPool(device, &pool_info, nullptr, &command_pool) != VK_SUCCESS)
        return result::err("Failed to create command pool");

    // Create transfer command pool
    VkCommandPoolCreateInfo transfer_pool_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = native_def.transfer_family.value(),
    };
    VkCommandPool transfer_command_pool;
    if (vkCreateCommandPool(device, &transfer_pool_info, nullptr, &transfer_command_pool) != VK_SUCCESS)
        return result::err("Failed to create transfer command pool");

    // Create sync context
    auto sync_context = GET_OR_FORWARD(vulkan_sync_context::create(device, init.config));

    // Create memory context
    auto memory_context = GET_OR_FORWARD(vulkan_memory_context::create(init.instance, device, physical_device));

    vulkan_device_state state = {
        .device = device,
        .graphics_queue = graphics_queue,
        .present_queue = present_queue,
        .transfer_queue = transfer_queue,
        .command_pool = command_pool,
        .transfer_command_pool = transfer_command_pool,
        .sync_context = std::move(sync_context),
        .memory_context = std::move(memory_context),
    };

    return result::ok(new vulkan_device(init, state));
}

result::ptr<graphics_swapchain> vulkan_device::create_swapchain(uint32_t width, uint32_t height) {
    vulkan_swapchain_init init = {
        .device = _device,
        .surface = _surface,
        .def = (const vulkan_device_def&) def(),
        .sync_context = *_sync_context,
        .memory_context = *_memory_context,
        .width = width,
        .height = height,
    };

    return vulkan_swapchain::create(init);
}

result::ptr<graphics_shader> vulkan_device::create_shader(std::unique_ptr<shader_binary> binary) {
    return vulkan_shader::create(std::move(binary), _device);
}

result::ptr<graphics_render_pass> vulkan_device::create_render_pass(const graphics_swapchain& swapchain) {
    auto& native_swapchain = (vulkan_swapchain&) swapchain;
    return vulkan_render_pass::create(native_swapchain, _device);
}

result::ptr<graphics_resource_layout>
vulkan_device::create_resource_layout(const std::vector<const graphics_shader*>& stages) {
    return vulkan_resource_layout::create(stages, _device);
}

result::ptr<graphics_resource_set> vulkan_device::create_resource_set(const graphics_resource_layout& layout,
                                                                      resource_set_ref ref) {
    vulkan_resource_set_init init = {
        .device = _device,
        .layout = (const vulkan_resource_layout&) layout,
        .ref = ref,
        .sync_context = *_sync_context,
    };

    return vulkan_resource_set::create(init);
}

result::ptr<graphics_pipeline> vulkan_device::create_pipeline(const graphics_pipeline_init& init) {
    return vulkan_pipeline::create(init, _device);
}

result::ptr<graphics_buffer> vulkan_device::create_buffer(buffer_usage_flags usage, uint32_t size) {
    vulkan_buffer_init init = {
        .device = _device,
        .usage = usage,
        .size = size,
        .def = (const vulkan_device_def&) def(),
        .memory_context = *_memory_context,
        .transfer_command_pool = _transfer_command_pool,
        .transfer_queue = _transfer_queue,
    };

    return vulkan_buffer::create(init);
}

result::ptr<graphics_image> vulkan_device::create_image(uint32_t width, uint32_t height, graphics_image_format format) {
    vulkan_image_init init = {
        .width = width,
        .height = height,
        .format = format,
        .device = _device,
        .def = (const vulkan_device_def*) &def(),
        .memory_context = _memory_context.get(),
        .transfer_command_pool = _transfer_command_pool,
        .transfer_queue = _transfer_queue,
    };

    return vulkan_image::create(init);
}

result::ptr<graphics_sampler> vulkan_device::create_sampler(const graphics_sampler_init& init) {
    return vulkan_sampler::create(init, _device);
}

result::ptr<graphics_uniform_buffer> vulkan_device::create_uniform_buffer(const shader_variable_type& type) {
    return vulkan_uniform_buffer::create(type, _device, *_sync_context, *_memory_context);
}

result::ptr<graphics_command_buffer> vulkan_device::create_command_buffer() {
    return vulkan_command_buffer::create(_device, _command_pool, *_sync_context);
}

void vulkan_device::submit_command_buffer(const graphics_command_buffer& command_buffer) {
    const auto& native_command_buffer = (const vulkan_command_buffer&) command_buffer;

    VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

    VkCommandBuffer vk_buffer = native_command_buffer.command_buffer();
    VkFence fence = _sync_context->gpu_wait_fence();
    VkSemaphore image_available_semaphore = _sync_context->image_available_semaphore();
    VkSemaphore render_finished_semaphore = _sync_context->render_finished_semaphore();

    VkSubmitInfo submit_info = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &image_available_semaphore,
        .pWaitDstStageMask = wait_stages,
        .commandBufferCount = 1,
        .pCommandBuffers = &vk_buffer,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &render_finished_semaphore,
    };

    if (vkQueueSubmit(_graphics_queue, 1, &submit_info, fence) != VK_SUCCESS)
        throw std::runtime_error("Failed to submit command buffer");
}

void vulkan_device::present(graphics_swapchain& swapchain) {
    auto& native_swapchain = (vulkan_swapchain&) swapchain;
    VkSwapchainKHR vk_swapchain = native_swapchain.swapchain();
    uint32_t image_index = native_swapchain.current_index();
    VkSemaphore render_finished_semaphore = _sync_context->render_finished_semaphore();

    VkPresentInfoKHR present_info = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &render_finished_semaphore,
        .swapchainCount = 1,
        .pSwapchains = &vk_swapchain,
        .pImageIndices = &image_index,
    };

    VkResult result = vkQueuePresentKHR(_present_queue, &present_info);
    native_swapchain.recreate_if_needed(result);
}

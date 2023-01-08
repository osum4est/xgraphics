#include "vulkan_swapchain.h"
#include "vulkan_memory_context.h"

#include <array>
#include <vector>
#include <vulkan/vulkan_core.h>

vulkan_swapchain::vulkan_swapchain(const vulkan_swapchain_init& init, const vulkan_swapchain_state& state)
    : graphics_swapchain(state.extent.width, state.extent.height),
      _device(init.device),
      _def(init.def),
      _surface(init.surface),
      _sync_context(init.sync_context),
      _memory_context(init.memory_context),
      _state(state) { }

vulkan_swapchain::~vulkan_swapchain() {
    destroy();
}

result::ptr<graphics_swapchain> vulkan_swapchain::create(const vulkan_swapchain_init& init) {
    VkPhysicalDevice physical_device = init.def.physical_device;

    // Pick a surface format
    uint32_t format_count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, init.surface, &format_count, nullptr);
    std::vector<VkSurfaceFormatKHR> formats(format_count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, init.surface, &format_count, formats.data());
    VkSurfaceFormatKHR format = formats[0];
    for (const auto& surface_format : formats) {
        if (surface_format.format == VK_FORMAT_B8G8R8A8_SRGB &&
            surface_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            format = surface_format;
            break;
        }
    }

    auto state = GET_OR_FORWARD(create_swapchain(init, format));

    return result::ok(new vulkan_swapchain(init, state));
}

void vulkan_swapchain::create_framebuffers(VkRenderPass render_pass) {
    auto& framebuffers = _framebuffers[render_pass];
    framebuffers.clear();
    for (const auto& view : image_views()) {
        std::array<VkImageView, 2> attachments = {view, _state.depth_image_view};
        VkFramebufferCreateInfo framebuffer_info = {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass = render_pass,
            .attachmentCount = (uint32_t) attachments.size(),
            .pAttachments = attachments.data(),
            .width = _state.extent.width,
            .height = _state.extent.height,
            .layers = 1,
        };

        VkFramebuffer framebuffer;
        if (vkCreateFramebuffer(_device, &framebuffer_info, nullptr, &framebuffer) != VK_SUCCESS)
            throw std::runtime_error("Failed to create framebuffer");

        framebuffers.push_back(framebuffer);
    }
}

void vulkan_swapchain::destroy_framebuffers(VkRenderPass render_pass) {
    auto& framebuffers = _framebuffers.at(render_pass);
    for (const auto& framebuffer : framebuffers)
        vkDestroyFramebuffer(_device, framebuffer, nullptr);
    _framebuffers.erase(render_pass);
}

void vulkan_swapchain::recreate_if_needed(VkResult result) {
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || _needs_recreate) recreate();
}

VkSwapchainKHR vulkan_swapchain::swapchain() const {
    return _state.swapchain;
}

const std::vector<VkImageView>& vulkan_swapchain::image_views() const {
    return _state.image_views;
}

VkFormat vulkan_swapchain::format() const {
    return _state.format.format;
}

VkExtent2D vulkan_swapchain::extent() const {
    return _state.extent;
}

uint32_t vulkan_swapchain::current_index() const {
    return _current_index;
}

VkFramebuffer vulkan_swapchain::current_framebuffer(VkRenderPass render_pass) const {
    return _framebuffers.at(render_pass)[_current_index];
}

void vulkan_swapchain::swap() {
    VkResult result = vkAcquireNextImageKHR(_device, _state.swapchain, std::numeric_limits<uint64_t>::max(),
                                            _sync_context.image_available_semaphore(), VK_NULL_HANDLE, &_current_index);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) _needs_recreate = true;
    else if (result != VK_SUCCESS) throw std::runtime_error("Failed to acquire swapchain image");
}

void vulkan_swapchain::resize(uint32_t width, uint32_t height) {
    graphics_swapchain::resize(width, height);
    _needs_recreate = true;
    _resized_extent = {width, height};
}

void vulkan_swapchain::recreate() {
    _needs_recreate = false;
    vkDeviceWaitIdle(_device);

    // First, check which render passes have framebuffers, so we can recreate them
    std::vector<VkRenderPass> render_passes;
    for (const auto& [render_pass, framebuffers] : _framebuffers)
        render_passes.push_back(render_pass);

    destroy();

    _state = create_swapchain(
                 vulkan_swapchain_init {
                     .device = _device,
                     .surface = _surface,
                     .def = _def,
                     .sync_context = _sync_context,
                     .memory_context = _memory_context,
                     .width = _resized_extent.width,
                     .height = _resized_extent.height,
                 },
                 _state.format)
                 .get();

    for (const auto& render_pass : render_passes)
        create_framebuffers(render_pass);
}

void vulkan_swapchain::destroy() {
    for (auto& [render_pass, framebuffers] : _framebuffers)
        for (const auto& framebuffer : framebuffers)
            vkDestroyFramebuffer(_device, framebuffer, nullptr);
    for (const auto& image_view : _state.image_views)
        vkDestroyImageView(_device, image_view, nullptr);
    vkDestroySwapchainKHR(_device, _state.swapchain, nullptr);

    _memory_context.destroy_image(_state.depth_image);
    vkDestroyImageView(_device, _state.depth_image_view, nullptr);

    _framebuffers.clear();
    _state.image_views.clear();
    _state.images.clear();
}

result::val<vulkan_swapchain_state> vulkan_swapchain::create_swapchain(const vulkan_swapchain_init& init,
                                                                       VkSurfaceFormatKHR format) {
    VkDevice device = init.device;
    VkSurfaceKHR surface = init.surface;
    const vulkan_device_def& def = init.def;
    VkPhysicalDevice physical_device = def.physical_device;

    // Get surface extent
    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &capabilities);
    VkExtent2D extent = {init.width, init.height};
    if (capabilities.currentExtent.width != UINT32_MAX) extent = capabilities.currentExtent;
    else {
        extent.width =
            std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, extent.width));
        extent.height =
            std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, extent.height));
    }

    // Pick number of images
    VkSurfaceCapabilitiesKHR surface_capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, init.surface, &surface_capabilities);
    uint32_t min_image_count = surface_capabilities.minImageCount + 1;
    if (surface_capabilities.maxImageCount > 0 && min_image_count > surface_capabilities.maxImageCount)
        min_image_count = surface_capabilities.maxImageCount;

    // Create swapchain
    VkSwapchainCreateInfoKHR create_info = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = surface,
        .minImageCount = min_image_count,
        .imageFormat = format.format,
        .imageColorSpace = format.colorSpace,
        .imageExtent = extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = nullptr,
    };

    if (def.graphics_family != def.present_family) {
        uint32_t queue_family_indices[] = {def.graphics_family.value(), def.present_family.value()};
        create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        create_info.queueFamilyIndexCount = 2;
        create_info.pQueueFamilyIndices = queue_family_indices;
    }

    create_info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    create_info.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    create_info.clipped = VK_TRUE;
    create_info.oldSwapchain = VK_NULL_HANDLE;

    VkSwapchainKHR swapchain;
    if (vkCreateSwapchainKHR(device, &create_info, nullptr, &swapchain) != VK_SUCCESS)
        return result::err("Failed to create swapchain");

    // Get swapchain images
    uint32_t image_count;
    vkGetSwapchainImagesKHR(device, swapchain, &image_count, nullptr);
    std::vector<VkImage> images(image_count);
    vkGetSwapchainImagesKHR(device, swapchain, &image_count, images.data());

    // Create image views
    std::vector<VkImageView> image_views;
    for (const auto& image : images)
        image_views.push_back(
            GET_OR_FORWARD(create_image_view(device, image, format.format, VK_IMAGE_ASPECT_COLOR_BIT)));

    // Create depth image
    VkImageCreateInfo image_create_info = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = VK_FORMAT_D32_SFLOAT,
        .extent = {.width = extent.width, .height = extent.height, .depth = 1},
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = nullptr,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    };
    auto depth_image = GET_OR_FORWARD(init.memory_context.create_gpu_image(image_create_info));
    auto depth_image_view =
        GET_OR_FORWARD(create_image_view(device, depth_image, VK_FORMAT_D32_SFLOAT, VK_IMAGE_ASPECT_DEPTH_BIT));

    return result::ok(vulkan_swapchain_state {
        .swapchain = swapchain,
        .format = format,
        .extent = extent,
        .images = images,
        .image_views = image_views,
        .depth_image = depth_image,
        .depth_image_view = depth_image_view,
    });
}

result::val<VkImageView> vulkan_swapchain::create_image_view(VkDevice device, VkImage image, VkFormat format,
                                                             VkImageAspectFlags aspect_flags) {
    VkImageViewCreateInfo view_create_info = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = image,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = format,
        .subresourceRange =
            {
                .aspectMask = aspect_flags,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
    };

    VkImageView image_view;
    if (vkCreateImageView(device, &view_create_info, nullptr, &image_view) != VK_SUCCESS)
        return result::err("Failed to create image view");

    return result::ok(image_view);
}
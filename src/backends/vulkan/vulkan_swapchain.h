#ifndef XGRAPHICS_VULKAN_SWAPCHAIN_H
#define XGRAPHICS_VULKAN_SWAPCHAIN_H

#include "vulkan_device_def.h"
#include "vulkan_memory_context.h"
#include "vulkan_sync_context.h"
#include <result/result.h>
#include <unordered_map>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include <xgraphics/xgraphics.h>

struct vulkan_swapchain_init {
    VkDevice device;
    VkSurfaceKHR surface;
    const vulkan_device_def& def;
    vulkan_sync_context& sync_context;
    vulkan_memory_context& memory_context;
    uint32_t width;
    uint32_t height;
};

struct vulkan_swapchain_state {
    VkSwapchainKHR swapchain;
    VkSurfaceFormatKHR format;
    VkExtent2D extent;
    std::vector<VkImage> images;
    std::vector<VkImageView> image_views;
    VkImage depth_image;
    VkImageView depth_image_view;
};

class vulkan_swapchain : public graphics_swapchain {
    VkDevice _device;
    VkSurfaceKHR _surface;
    const vulkan_device_def& _def;
    vulkan_sync_context& _sync_context;
    vulkan_memory_context& _memory_context;

    vulkan_swapchain_state _state;

    std::unordered_map<VkRenderPass, std::vector<VkFramebuffer>> _framebuffers;
    uint32_t _current_index = 0;
    bool _needs_recreate = false;
    VkExtent2D _resized_extent = {};

  public:
    vulkan_swapchain(const vulkan_swapchain_init& init, const vulkan_swapchain_state& state);
    ~vulkan_swapchain() override;

    static result::ptr<graphics_swapchain> create(const vulkan_swapchain_init& init);

    void create_framebuffers(VkRenderPass render_pass);
    void destroy_framebuffers(VkRenderPass render_pass);
    void recreate_if_needed(VkResult result);

    [[nodiscard]] VkSwapchainKHR swapchain() const;
    [[nodiscard]] const std::vector<VkImageView>& image_views() const;
    [[nodiscard]] VkFormat format() const;
    [[nodiscard]] VkExtent2D extent() const;
    [[nodiscard]] uint32_t current_index() const;
    [[nodiscard]] VkFramebuffer current_framebuffer(VkRenderPass render_pass) const;

    void swap() override;
    void resize(uint32_t width, uint32_t height) override;

  private:
    void recreate();
    void destroy();

    static result::val<vulkan_swapchain_state> create_swapchain(const vulkan_swapchain_init& init,
                                                                VkSurfaceFormatKHR format);
    static result::val<VkImageView> create_image_view(VkDevice device, VkImage image, VkFormat format,
                                                      VkImageAspectFlags aspect_flags);
};

#endif

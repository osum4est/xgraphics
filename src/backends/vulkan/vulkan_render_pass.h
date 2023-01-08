#ifndef XGRAPHICS_VULKAN_RENDER_PASS_H
#define XGRAPHICS_VULKAN_RENDER_PASS_H

#include "vulkan_swapchain.h"
#include <array>
#include <result/result.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include <xgraphics/interfaces/graphics_render_pass.h>

class vulkan_render_pass : public graphics_render_pass {
    VkDevice _device;
    VkRenderPass _render_pass;
    VkClearValue _clear_color = {};
    std::array<VkClearValue, 2> _clear_values = {
        VkClearValue {.color = {.float32 = {0.0f, 0.0f, 0.0f, 1.0f}}},
        VkClearValue {.depthStencil = {.depth = 1.0f, .stencil = 0}},
    };

    explicit vulkan_render_pass(const vulkan_swapchain& swapchain, VkDevice device, VkRenderPass render_pass);

  public:
    ~vulkan_render_pass() override;

    static result::ptr<graphics_render_pass> create(vulkan_swapchain& swapchain, VkDevice device);

    void set_clear_color(uint32_t clear_color) override;

    [[nodiscard]] VkRenderPass render_pass() const;
    [[nodiscard]] const VkClearValue* vk_clear_values() const;
    [[nodiscard]] uint32_t vk_clear_values_count() const;
};

#endif

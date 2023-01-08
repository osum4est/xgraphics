#include "vulkan_render_pass.h"
#include "../common/xgraphics_utils.h"
#include "vulkan_swapchain.h"
#include <cmath>
#include <vector>

vulkan_render_pass::vulkan_render_pass(const vulkan_swapchain& swapchain, VkDevice device, VkRenderPass render_pass)
    : graphics_render_pass(swapchain), _device(device), _render_pass(render_pass) { }

vulkan_render_pass::~vulkan_render_pass() {
    auto& native_swapchain = (vulkan_swapchain&) swapchain();
    native_swapchain.destroy_framebuffers(_render_pass);
    vkDestroyRenderPass(_device, _render_pass, nullptr);
}

result::ptr<graphics_render_pass> vulkan_render_pass::create(vulkan_swapchain& swapchain, VkDevice device) {
    // Create color attachment
    VkAttachmentDescription color_attachment = {
        .format = swapchain.format(),
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
    };

    VkAttachmentReference color_attachment_ref = {
        .attachment = 0,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    };

    // Create depth attachment
    VkAttachmentDescription depth_attachment = {
        .format = VK_FORMAT_D32_SFLOAT,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
    };

    VkAttachmentReference depth_attachment_ref = {
        .attachment = 1,
        .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
    };

    // Create subpass
    VkSubpassDescription subpass = {
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .colorAttachmentCount = 1,
        .pColorAttachments = &color_attachment_ref,
        .pDepthStencilAttachment = &depth_attachment_ref,
    };

    VkSubpassDependency dependency = {
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,
        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
        .srcAccessMask = 0,
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
    };

    std::vector<VkAttachmentDescription> attachments = {color_attachment, depth_attachment};
    VkRenderPassCreateInfo render_pass_info = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = (uint32_t) attachments.size(),
        .pAttachments = attachments.data(),
        .subpassCount = 1,
        .pSubpasses = &subpass,
        .dependencyCount = 1,
        .pDependencies = &dependency,
    };

    VkRenderPass render_pass;
    if (vkCreateRenderPass(device, &render_pass_info, nullptr, &render_pass) != VK_SUCCESS)
        return result::err("Failed to create render pass");

    // Create framebuffers
    swapchain.create_framebuffers(render_pass);

    return result::ok(new vulkan_render_pass(swapchain, device, render_pass));
}

void vulkan_render_pass::set_clear_color(uint32_t clear_color) {
    graphics_render_pass::set_clear_color(clear_color);

    _clear_color = {{{
        xgraphics_utils::srgb_to_linear((float) (clear_color >> 16 & 0xFF) / 255.0f),
        xgraphics_utils::srgb_to_linear((float) (clear_color >> 8 & 0xFF) / 255.0f),
        xgraphics_utils::srgb_to_linear((float) (clear_color & 0xFF) / 255.0f),
        (float) (clear_color >> 24 & 0xFF) / 255.0f,
    }}};

    _clear_values[0] = _clear_color;
}

VkRenderPass vulkan_render_pass::render_pass() const {
    return _render_pass;
}

const VkClearValue* vulkan_render_pass::vk_clear_values() const {
    return _clear_values.data();
}

uint32_t vulkan_render_pass::vk_clear_values_count() const {
    return _clear_values.size();
}

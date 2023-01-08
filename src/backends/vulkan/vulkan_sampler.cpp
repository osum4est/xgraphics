#include "vulkan_sampler.h"

vulkan_sampler::vulkan_sampler(const graphics_sampler_init& init, VkDevice device, VkSampler sampler)
    : graphics_sampler(init), _device(device), _sampler(sampler) { }

vulkan_sampler::~vulkan_sampler() {
    vkDestroySampler(_device, _sampler, nullptr);
}

result::ptr<graphics_sampler> vulkan_sampler::create(const graphics_sampler_init& init, VkDevice device) {
    VkSamplerCreateInfo sampler_info = {
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .magFilter = vk_filter(init.mag_filter).get(),
        .minFilter = vk_filter(init.min_filter).get(),
        .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
        .addressModeU = vk_address_mode(init.address_mode.u).get(),
        .addressModeV = vk_address_mode(init.address_mode.v).get(),
        .addressModeW = vk_address_mode(init.address_mode.w).get(),
        .mipLodBias = 0.0f,
        .anisotropyEnable = VK_TRUE,
        .maxAnisotropy = init.max_anisotropy,
        .compareEnable = VK_FALSE,
        .compareOp = VK_COMPARE_OP_ALWAYS,
        .minLod = 0.0f,
        .maxLod = 0.0f,
        .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
        .unnormalizedCoordinates = VK_FALSE,
    };

    VkSampler sampler;
    if (vkCreateSampler(device, &sampler_info, nullptr, &sampler) != VK_SUCCESS)
        return result::err("failed to create texture sampler!");

    return result::ok(new vulkan_sampler(init, device, sampler));
}

VkSampler vulkan_sampler::sampler() const {
    return _sampler;
}

result::val<VkFilter> vulkan_sampler::vk_filter(graphics_sampler_filter filter) {
    switch (filter) {
        case graphics_sampler_filter::nearest:
            return result::ok(VK_FILTER_NEAREST);
        case graphics_sampler_filter::linear:
            return result::ok(VK_FILTER_LINEAR);
    }
    return result::err("Invalid filter");
}

result::val<VkSamplerAddressMode> vulkan_sampler::vk_address_mode(graphics_sampler_address_mode address_mode) {
    switch (address_mode) {
        case graphics_sampler_address_mode::repeat:
            return result::ok(VK_SAMPLER_ADDRESS_MODE_REPEAT);
        case graphics_sampler_address_mode::mirror_repeat:
            return result::ok(VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT);
        case graphics_sampler_address_mode::clamp_to_edge:
            return result::ok(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
        case graphics_sampler_address_mode::mirror_clamp_to_edge:
            return result::ok(VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE);
        case graphics_sampler_address_mode::clamp_to_border:
            return result::ok(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER);
    }
    return result::err("Invalid address mode");
}

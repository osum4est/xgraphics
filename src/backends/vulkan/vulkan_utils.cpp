#include "vulkan_utils.h"

result::val<VkShaderStageFlagBits> vulkan_utils::vk_shader_stage(const shader_kind& kind) {
    switch (kind) {
        case shader_kind::vertex:
            return result::ok(VK_SHADER_STAGE_VERTEX_BIT);
        case shader_kind::fragment:
            return result::ok(VK_SHADER_STAGE_FRAGMENT_BIT);
        default:
            return result::err("Unsupported shader stage");
    }
}

result::val<VkDescriptorType> vulkan_utils::vk_descriptor_type(const shader_variable_type& type) {
    switch (type.base_type) {
        case shader_variable_base_type::image:
            return result::ok(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
        case shader_variable_base_type::sampler:
            return result::ok(VK_DESCRIPTOR_TYPE_SAMPLER);
        case shader_variable_base_type::sampled_image:
            return result::ok(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
        case shader_variable_base_type::structure:
            return result::ok(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
        default:
            return result::err("Unsupported descriptor type");
    }
}

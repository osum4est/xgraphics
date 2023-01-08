#ifndef XGRAPHICS_VULKAN_UTILS_H
#define XGRAPHICS_VULKAN_UTILS_H

#include <result/result.h>
#include <vulkan/vulkan.h>
#include <xgraphics/shaders/shader_data.h>

class vulkan_utils {
  public:
    // TODO: Put more conversion functions here
    static result::val<VkShaderStageFlagBits> vk_shader_stage(const shader_kind& kind);
    static result::val<VkDescriptorType> vk_descriptor_type(const shader_variable_type& type);
};

#endif

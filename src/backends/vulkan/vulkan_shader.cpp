#include "vulkan_shader.h"
#include "vulkan_utils.h"

#include <vector>

vulkan_shader::vulkan_shader(std::unique_ptr<shader_binary> binary, VkDevice device, VkShaderModule module)
    : graphics_shader(std::move(binary)), _device(device), _module(module) { }

vulkan_shader::~vulkan_shader() {
    vkDestroyShaderModule(_device, _module, nullptr);
}

result::ptr<graphics_shader> vulkan_shader::create(std::unique_ptr<shader_binary> binary, VkDevice device) {
    VkShaderModuleCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = binary->data().size(),
        .pCode = (const uint32_t*) binary->data().data(),
    };

    VkShaderModule module;
    if (vkCreateShaderModule(device, &create_info, nullptr, &module) != VK_SUCCESS)
        return result::err("Failed to create shader module");

    return result::ok(new vulkan_shader(std::move(binary), device, module));
}

[[nodiscard]] VkShaderModule vulkan_shader::module() const {
    return _module;
}
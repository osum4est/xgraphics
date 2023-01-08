#ifndef XGRAPHICS_VULKAN_SHADER_H
#define XGRAPHICS_VULKAN_SHADER_H

#include <result/result.h>
#include <vulkan/vulkan.h>
#include <xgraphics/interfaces/graphics_shader.h>

class vulkan_shader : public graphics_shader {
    VkDevice _device;
    VkShaderModule _module;

    vulkan_shader(std::unique_ptr<shader_binary> binary, VkDevice device, VkShaderModule module);

  public:
    ~vulkan_shader() override;

    static result::ptr<graphics_shader> create(std::unique_ptr<shader_binary> binary, VkDevice device);

    [[nodiscard]] VkShaderModule module() const;
};

#endif

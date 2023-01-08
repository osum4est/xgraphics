#ifndef XGRAPHICS_VULKAN_RESOURCE_LAYOUT_H
#define XGRAPHICS_VULKAN_RESOURCE_LAYOUT_H

#include <vulkan/vulkan.h>
#include <xgraphics/interfaces/graphics_resource_layout.h>

class vulkan_resource_layout : public graphics_resource_layout {
    typedef std::unordered_map<uint32_t, VkDescriptorSetLayout> vk_set_layouts;

    VkDevice _device;
    VkPipelineLayout _layout;
    vk_set_layouts _set_layouts;

  protected:
    explicit vulkan_resource_layout(const std::vector<const graphics_shader*>& stages, VkDevice device,
                                    VkPipelineLayout layout, const vk_set_layouts& set_layouts);

  public:
    ~vulkan_resource_layout() override;
    static result::ptr<graphics_resource_layout> create(const std::vector<const graphics_shader*>& stages,
                                                        VkDevice device);

    [[nodiscard]] VkPipelineLayout layout() const;
    [[nodiscard]] VkDescriptorSetLayout set_layout(resource_set_ref ref) const;
};

#endif

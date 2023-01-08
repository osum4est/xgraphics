#include "vulkan_resource_layout.h"
#include "vulkan_utils.h"

vulkan_resource_layout::vulkan_resource_layout(const std::vector<const graphics_shader*>& stages, VkDevice device,
                                               VkPipelineLayout layout, const vk_set_layouts& set_layouts)
    : graphics_resource_layout(stages), _device(device), _layout(layout), _set_layouts(set_layouts) { }

vulkan_resource_layout::~vulkan_resource_layout() {
    for (auto& [_, set_layout] : _set_layouts)
        vkDestroyDescriptorSetLayout(_device, set_layout, nullptr);
    vkDestroyPipelineLayout(_device, _layout, nullptr);
}

result::ptr<graphics_resource_layout> vulkan_resource_layout::create(const std::vector<const graphics_shader*>& stages,
                                                                     VkDevice device) {
    auto merged = merge_resources(stages);

    // Create descriptor set layouts
    vk_set_layouts set_layouts;
    std::vector<VkDescriptorSetLayout> set_layouts_array;
    for (const auto& set : merged->resource_sets) {

        // Create descriptor set bindings
        std::vector<VkDescriptorSetLayoutBinding> descriptor_bindings;
        for (const auto& resource : set.resources) {
            VkDescriptorSetLayoutBinding layout_binding = {
                .binding = resource.backend_binding,
                .descriptorType = vulkan_utils::vk_descriptor_type(*resource.type).get(),
                .descriptorCount =
                    resource.type->array_sizes.empty() ? 1 : resource.type->array_sizes.back(), // TODO: back or front?
                .stageFlags = 0,
            };

            for (const auto& stage : resource.stages)
                layout_binding.stageFlags |= vulkan_utils::vk_shader_stage(stage->info().kind).get();

            descriptor_bindings.push_back(layout_binding);
        }

        // Create descriptor set layout
        VkDescriptorSetLayoutCreateInfo layout_info = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .bindingCount = (uint32_t) descriptor_bindings.size(),
            .pBindings = descriptor_bindings.data(),
        };

        VkDescriptorSetLayout set_layout;
        if (vkCreateDescriptorSetLayout(device, &layout_info, nullptr, &set_layout) != VK_SUCCESS)
            return result::err("Failed to create descriptor set layout");

        set_layouts[set.backend_number] = set_layout;
        set_layouts_array.push_back(set_layout);
    }

    // Create pipeline layout
    VkPipelineLayoutCreateInfo pipeline_layout_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = (uint32_t) set_layouts_array.size(),
        .pSetLayouts = set_layouts_array.data(),
        .pushConstantRangeCount = 0,
        .pPushConstantRanges = nullptr,
    };

    VkPipelineLayout pipeline_layout;
    if (vkCreatePipelineLayout(device, &pipeline_layout_info, nullptr, &pipeline_layout) != VK_SUCCESS)
        return result::err("Failed to create pipeline layout");

    return result::ok(new vulkan_resource_layout(stages, device, pipeline_layout, set_layouts));
}

VkPipelineLayout vulkan_resource_layout::layout() const {
    return _layout;
}

VkDescriptorSetLayout vulkan_resource_layout::set_layout(resource_set_ref ref) const {
    return _set_layouts.at(ref->backend_number);
}

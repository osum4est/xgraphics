#ifndef XGRAPHICS_VULKAN_RESOURCE_SET_H
#define XGRAPHICS_VULKAN_RESOURCE_SET_H

#include "vulkan_resource_layout.h"
#include "vulkan_sync_context.h"
#include <vulkan/vulkan.h>
#include <xgraphics/interfaces/graphics_resource_set.h>

struct vulkan_resource_set_init {
    VkDevice device;
    const vulkan_resource_layout& layout;
    resource_set_ref ref;
    const vulkan_sync_context& sync_context;
};

class vulkan_resource_set : public graphics_resource_set {
    VkDevice _device;
    VkDescriptorPool _pool;
    VkPipelineLayout _pipeline_layout;
    std::vector<VkDescriptorSet> _descriptor_sets;
    const vulkan_sync_context* _sync_context;

    vulkan_resource_set(const vulkan_resource_set_init& init, VkDescriptorPool pool,
                        const std::vector<VkDescriptorSet>& descriptor_sets);

  public:
    ~vulkan_resource_set() override;

    static result::ptr<graphics_resource_set> create(const vulkan_resource_set_init& init);

    void bind_uniform_buffer(resource_binding_ref binding, const graphics_uniform_buffer& buffer) override;
    void bind_sampled_image(resource_binding_ref binding, const graphics_image& image,
                            const graphics_sampler& sampler) override;

    [[nodiscard]] VkPipelineLayout pipeline_layout() const;
    [[nodiscard]] VkDescriptorSet descriptor_set() const;
};

#endif

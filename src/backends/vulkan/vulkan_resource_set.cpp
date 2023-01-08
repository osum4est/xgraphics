#include "vulkan_resource_set.h"
#include "vulkan_image.h"
#include "vulkan_sampler.h"
#include "vulkan_uniform_buffer.h"
#include "vulkan_utils.h"

vulkan_resource_set::vulkan_resource_set(const vulkan_resource_set_init& init, VkDescriptorPool pool,
                                         const std::vector<VkDescriptorSet>& descriptor_sets)
    : graphics_resource_set(init.layout, init.ref),
      _device(init.device),
      _pool(pool),
      _pipeline_layout(((const vulkan_resource_layout&) init.layout).layout()),
      _descriptor_sets(descriptor_sets),
      _sync_context(&init.sync_context) { }

vulkan_resource_set::~vulkan_resource_set() {
    vkDestroyDescriptorPool(_device, _pool, nullptr);
}

result::ptr<graphics_resource_set> vulkan_resource_set::create(const vulkan_resource_set_init& init) {
    auto ref = init.ref;
    auto frames = init.sync_context.frames_in_flight();

    // Create pools
    std::unordered_map<VkDescriptorType, VkDescriptorPoolSize> pool_sizes_map;
    for (const auto& resource : ref->resources) {
        VkDescriptorType type = vulkan_utils::vk_descriptor_type(*resource.type).get();
        pool_sizes_map[type].type = type;
        pool_sizes_map[type].descriptorCount += frames;
    }

    std::vector<VkDescriptorPoolSize> pool_sizes;
    for (const auto& [_, pool_size] : pool_sizes_map)
        pool_sizes.push_back(pool_size);

    VkDescriptorPoolCreateInfo pool_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .maxSets = (uint32_t) frames,
        .poolSizeCount = (uint32_t) pool_sizes.size(),
        .pPoolSizes = pool_sizes.data(),
    };

    VkDescriptorPool pool;
    if (vkCreateDescriptorPool(init.device, &pool_info, nullptr, &pool) != VK_SUCCESS)
        return result::err("Failed to create descriptor pool");

    // Create descriptor sets
    std::vector<VkDescriptorSetLayout> layouts(frames, init.layout.set_layout(ref));
    std::vector<VkDescriptorSet> descriptor_sets(frames);
    VkDescriptorSetAllocateInfo alloc_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = pool,
        .descriptorSetCount = (uint32_t) frames,
        .pSetLayouts = layouts.data(),
    };

    if (vkAllocateDescriptorSets(init.device, &alloc_info, descriptor_sets.data()) != VK_SUCCESS)
        return result::err("Failed to allocate descriptor sets");

    return result::ok(new vulkan_resource_set(init, pool, descriptor_sets));
}

void vulkan_resource_set::bind_uniform_buffer(resource_binding_ref binding, const graphics_uniform_buffer& buffer) {
    const auto& native_buffer = (const vulkan_uniform_buffer&) buffer;

    for (int i = 0; i < _sync_context->frames_in_flight(); i++) {
        VkDescriptorBufferInfo buffer_info = {
            .buffer = native_buffer.buffer(i),
            .offset = 0,
            .range = native_buffer.size(),
        };

        VkWriteDescriptorSet descriptor_write = {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = _descriptor_sets[i],
            .dstBinding = binding->backend_binding,
            .dstArrayElement = 0, // TODO: Arrays
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .pBufferInfo = &buffer_info,
        };

        vkUpdateDescriptorSets(_device, 1, &descriptor_write, 0, nullptr);
    }
}

void vulkan_resource_set::bind_sampled_image(resource_binding_ref binding, const graphics_image& image,
                                             const graphics_sampler& sampler) {
    const auto& native_image = (const vulkan_image&) image;
    const auto& native_sampler = (const vulkan_sampler&) sampler;

    for (int i = 0; i < _sync_context->frames_in_flight(); i++) {
        VkDescriptorImageInfo image_info = {
            .sampler = native_sampler.sampler(),
            .imageView = native_image.image_view(),
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        };

        VkWriteDescriptorSet descriptor_write = {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = _descriptor_sets[i],
            .dstBinding = binding->backend_binding,
            .dstArrayElement = 0, // TODO: Arrays
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .pImageInfo = &image_info,
        };

        vkUpdateDescriptorSets(_device, 1, &descriptor_write, 0, nullptr);
    }
}

VkPipelineLayout vulkan_resource_set::pipeline_layout() const {
    return _pipeline_layout;
}

VkDescriptorSet vulkan_resource_set::descriptor_set() const {
    return _descriptor_sets[_sync_context->current_frame()];
}

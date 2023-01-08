#ifndef XGRAPHICS_VULKAN_MEMORY_CONTEXT_H
#define XGRAPHICS_VULKAN_MEMORY_CONTEXT_H

#include <result/result.h>
#include <unordered_map>
#include <vk_mem_alloc.h>

class vulkan_memory_context {
    VmaAllocator _allocator;
    std::unordered_map<VkBuffer, VmaAllocation> _buffer_allocations;
    std::unordered_map<VkImage, VmaAllocation> _image_allocations;

    explicit vulkan_memory_context(VmaAllocator allocator);

  public:
    vulkan_memory_context(const vulkan_memory_context&) = delete;
    ~vulkan_memory_context();

    static result::ptr<vulkan_memory_context> create(VkInstance instance, VkDevice device,
                                                     VkPhysicalDevice physical_device);

    result::val<VkBuffer> create_staging_buffer(VkBufferCreateInfo buffer_info);
    result::val<VkBuffer> create_gpu_buffer(VkBufferCreateInfo buffer_info);
    result::val<VkBuffer> create_mapped_buffer(VkBufferCreateInfo buffer_info);
    result::val<VkImage> create_gpu_image(VkImageCreateInfo image_info);

    void destroy_buffer(VkBuffer buffer);
    void destroy_image(VkImage image);

    [[nodiscard]] void* map_buffer(VkBuffer buffer);
    void unmap_buffer(VkBuffer buffer);

  private:
    result::val<VkBuffer> create_buffer(VkBufferCreateInfo buffer_info, VmaAllocationCreateInfo allocation_info);
    result::val<VkImage> create_image(VkImageCreateInfo image_info, VmaAllocationCreateInfo allocation_info);
};

#endif

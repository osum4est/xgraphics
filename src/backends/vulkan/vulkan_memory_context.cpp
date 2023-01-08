#include "vulkan_memory_context.h"

vulkan_memory_context::vulkan_memory_context(VmaAllocator allocator) : _allocator(allocator) { }

vulkan_memory_context::~vulkan_memory_context() {
    vmaDestroyAllocator(_allocator);
}

result::ptr<vulkan_memory_context> vulkan_memory_context::create(VkInstance instance, VkDevice device,
                                                                 VkPhysicalDevice physical_device) {
    VmaAllocatorCreateInfo allocator_info = {
        .physicalDevice = physical_device,
        .device = device,
        .instance = instance,
    };

    VmaAllocator allocator;
    if (vmaCreateAllocator(&allocator_info, &allocator) != VK_SUCCESS)
        return result::err("Failed to create VMA allocator");

    return result::ok(new vulkan_memory_context(allocator));
}

result::val<VkBuffer> vulkan_memory_context::create_staging_buffer(VkBufferCreateInfo buffer_info) {
    VmaAllocationCreateInfo allocation_info = {
        .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
        .usage = VMA_MEMORY_USAGE_AUTO,
    };

    return create_buffer(buffer_info, allocation_info);
}

result::val<VkBuffer> vulkan_memory_context::create_gpu_buffer(VkBufferCreateInfo buffer_info) {
    // TODO: Make sure this gets created locally
    VmaAllocationCreateInfo allocation_info = {
        .usage = VMA_MEMORY_USAGE_AUTO,
    };

    return create_buffer(buffer_info, allocation_info);
}

result::val<VkBuffer> vulkan_memory_context::create_mapped_buffer(VkBufferCreateInfo buffer_info) {
    VmaAllocationCreateInfo allocation_info = {
        .flags = VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
        .usage = VMA_MEMORY_USAGE_AUTO,
    };

    return create_buffer(buffer_info, allocation_info);
}

result::val<VkImage> vulkan_memory_context::create_gpu_image(VkImageCreateInfo image_info) {
    VmaAllocationCreateInfo allocation_info = {
        .usage = VMA_MEMORY_USAGE_AUTO,
    };

    return create_image(image_info, allocation_info);
}

void vulkan_memory_context::destroy_buffer(VkBuffer buffer) {
    auto allocation = _buffer_allocations.at(buffer);
    vmaDestroyBuffer(_allocator, buffer, allocation);
    _buffer_allocations.erase(buffer);
}

void vulkan_memory_context::destroy_image(VkImage image) {
    auto allocation = _image_allocations.at(image);
    vmaDestroyImage(_allocator, image, allocation);
    _image_allocations.erase(image);
}

void* vulkan_memory_context::map_buffer(VkBuffer buffer) {
    auto allocation = _buffer_allocations.at(buffer);
    void* data;
    vmaMapMemory(_allocator, allocation, &data);
    return data;
}

void vulkan_memory_context::unmap_buffer(VkBuffer buffer) {
    auto allocation = _buffer_allocations.at(buffer);
    vmaUnmapMemory(_allocator, allocation);
}

result::val<VkBuffer> vulkan_memory_context::create_buffer(VkBufferCreateInfo buffer_info,
                                                           VmaAllocationCreateInfo allocation_info) {
    VkBuffer buffer;
    VmaAllocation allocation;
    if (vmaCreateBuffer(_allocator, &buffer_info, &allocation_info, &buffer, &allocation, nullptr) != VK_SUCCESS)
        return result::err("Failed to create buffer");

    _buffer_allocations[buffer] = allocation;
    return result::ok(buffer);
}

result::val<VkImage> vulkan_memory_context::create_image(VkImageCreateInfo image_info,
                                                         VmaAllocationCreateInfo allocation_info) {
    VkImage image;
    VmaAllocation allocation;
    if (vmaCreateImage(_allocator, &image_info, &allocation_info, &image, &allocation, nullptr) != VK_SUCCESS)
        return result::err("Failed to create image");

    _image_allocations[image] = allocation;
    return result::ok(image);
}
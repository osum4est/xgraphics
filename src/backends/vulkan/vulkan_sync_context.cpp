#include "vulkan_sync_context.h"

#include <vector>

vulkan_sync_context::vulkan_sync_context(VkDevice device, const std::vector<VkFence>& gpu_wait_fences,
                                         const std::vector<VkSemaphore>& image_available_semaphore,
                                         const std::vector<VkSemaphore>& render_finished_semaphore,
                                         uint32_t frames_in_flight)
    : _device(device),
      _gpu_wait_fences(gpu_wait_fences),
      _image_available_semaphore(image_available_semaphore),
      _render_finished_semaphore(render_finished_semaphore),
      _frames_in_flight(frames_in_flight) { }

vulkan_sync_context::~vulkan_sync_context() {
    for (auto fence : _gpu_wait_fences)
        vkDestroyFence(_device, fence, nullptr);
    for (auto semaphore : _image_available_semaphore)
        vkDestroySemaphore(_device, semaphore, nullptr);
    for (auto semaphore : _render_finished_semaphore)
        vkDestroySemaphore(_device, semaphore, nullptr);
}

result::ptr<vulkan_sync_context> vulkan_sync_context::create(VkDevice device, graphics_config config) {
    int frames_in_flight = config.frames_in_flight;

    std::vector<VkFence> gpu_wait_fences;
    std::vector<VkSemaphore> image_available_semaphore;
    std::vector<VkSemaphore> render_finished_semaphore;

    for (int i = 0; i < frames_in_flight; i++) {
        VkFence fence;
        VkSemaphore image_available;
        VkSemaphore render_finished;

        VkFenceCreateInfo fence_info = {
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        };

        if (vkCreateFence(device, &fence_info, nullptr, &fence) != VK_SUCCESS)
            return result::err("Failed to create fence");

        VkSemaphoreCreateInfo semaphore_info = {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        };

        if (vkCreateSemaphore(device, &semaphore_info, nullptr, &image_available) != VK_SUCCESS)
            return result::err("Failed to create semaphore");

        if (vkCreateSemaphore(device, &semaphore_info, nullptr, &render_finished) != VK_SUCCESS)
            return result::err("Failed to create semaphore");

        gpu_wait_fences.push_back(fence);
        image_available_semaphore.push_back(image_available);
        render_finished_semaphore.push_back(render_finished);
    }

    return result::ok(new vulkan_sync_context(device, gpu_wait_fences, image_available_semaphore,
                                              render_finished_semaphore, config.frames_in_flight));
}

void vulkan_sync_context::set_current_frame(int current_frame) {
    _current_frame = current_frame;
}

VkFence vulkan_sync_context::gpu_wait_fence() const {
    return _gpu_wait_fences[_current_frame];
}

VkSemaphore vulkan_sync_context::image_available_semaphore() const {
    return _image_available_semaphore[_current_frame];
}

VkSemaphore vulkan_sync_context::render_finished_semaphore() const {
    return _render_finished_semaphore[_current_frame];
}

uint32_t vulkan_sync_context::current_frame() const {
    return _current_frame;
}

uint32_t vulkan_sync_context::frames_in_flight() const {
    return _frames_in_flight;
}

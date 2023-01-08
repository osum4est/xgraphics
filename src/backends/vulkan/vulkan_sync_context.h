#ifndef XGRAPHICS_VULKAN_SYNC_CONTEXT_H
#define XGRAPHICS_VULKAN_SYNC_CONTEXT_H

#include <result/result.h>
#include <vector>
#include <vulkan/vulkan.h>
#include <xgraphics/graphics_config.h>

class vulkan_sync_context {
    VkDevice _device;
    std::vector<VkFence> _gpu_wait_fences;
    std::vector<VkSemaphore> _image_available_semaphore;
    std::vector<VkSemaphore> _render_finished_semaphore;
    uint32_t _current_frame = 0;
    uint32_t _frames_in_flight;

    explicit vulkan_sync_context(VkDevice device, const std::vector<VkFence>& gpu_wait_fences,
                                 const std::vector<VkSemaphore>& image_available_semaphore,
                                 const std::vector<VkSemaphore>& render_finished_semaphore, uint32_t frames_in_flight);

  public:
    vulkan_sync_context(const vulkan_sync_context&) = delete;
    ~vulkan_sync_context();

    static result::ptr<vulkan_sync_context> create(VkDevice device, graphics_config config);

    void set_current_frame(int current_frame);

    [[nodiscard]] VkFence gpu_wait_fence() const;
    [[nodiscard]] VkSemaphore image_available_semaphore() const;
    [[nodiscard]] VkSemaphore render_finished_semaphore() const;
    [[nodiscard]] uint32_t current_frame() const;
    [[nodiscard]] uint32_t frames_in_flight() const;
};

#endif

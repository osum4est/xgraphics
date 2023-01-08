#ifndef XGRAPHICS_VULKAN_COMMAND_BUFFER_H
#define XGRAPHICS_VULKAN_COMMAND_BUFFER_H

#include "vulkan_pipeline.h"
#include "vulkan_sync_context.h"
#include <result/result.h>
#include <vulkan/vulkan.h>
#include <xgraphics/interfaces/graphics_buffer.h>
#include <xgraphics/interfaces/graphics_command_buffer.h>

class vulkan_command_buffer : public graphics_command_buffer {
    std::vector<VkCommandBuffer> _command_buffers;
    const vulkan_sync_context* _sync_context;

    explicit vulkan_command_buffer(const std::vector<VkCommandBuffer>& command_buffer,
                                   const vulkan_sync_context& sync_context);

  public:
    static result::ptr<graphics_command_buffer> create(VkDevice device, VkCommandPool command_pool,
                                                       const vulkan_sync_context& sync_context);

    [[nodiscard]] VkCommandBuffer command_buffer() const;

    void begin() override;
    void end() override;
    void begin_render_pass(const graphics_render_pass& render_pass) override;
    void end_render_pass() override;
    void bind_pipeline(const graphics_pipeline& pipeline) override;
    void bind_vertex_buffer(const graphics_buffer& buffer, uint32_t offset, int index) override;
    void bind_resource_set(const graphics_resource_set& resource_set) override;
    void draw(uint32_t vertex_start, uint32_t vertex_count, uint32_t instance_start, uint32_t instance_count) override;
    void draw_indexed(const graphics_buffer& index_buffer, uint32_t index_offset, index_type type, uint32_t index_start,
                      uint32_t index_count, uint32_t vertex_offset, uint32_t instance_start,
                      uint32_t instance_count) override;
};

#endif

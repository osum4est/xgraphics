#include "vulkan_command_buffer.h"
#include "vulkan_buffer.h"
#include "vulkan_pipeline.h"
#include "vulkan_render_pass.h"
#include "vulkan_resource_set.h"

vulkan_command_buffer::vulkan_command_buffer(const std::vector<VkCommandBuffer>& command_buffer,
                                             const vulkan_sync_context& sync_context)
    : _command_buffers(command_buffer), _sync_context(&sync_context) { }

result::ptr<graphics_command_buffer> vulkan_command_buffer::create(VkDevice device, VkCommandPool command_pool,
                                                                   const vulkan_sync_context& sync_context) {
    auto frames_in_flight = sync_context.frames_in_flight();
    VkCommandBufferAllocateInfo alloc_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = command_pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = frames_in_flight,
    };

    std::vector<VkCommandBuffer> command_buffers;
    command_buffers.resize(frames_in_flight);
    if (vkAllocateCommandBuffers(device, &alloc_info, command_buffers.data()) != VK_SUCCESS)
        return result::err("Failed to allocate command buffers");

    return result::ok(new vulkan_command_buffer(command_buffers, sync_context));
}

VkCommandBuffer vulkan_command_buffer::command_buffer() const {
    return _command_buffers[_sync_context->current_frame()];
}

void vulkan_command_buffer::begin() {
    vkResetCommandBuffer(command_buffer(), 0);

    VkCommandBufferBeginInfo begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = 0,
        .pInheritanceInfo = nullptr,
    };

    if (vkBeginCommandBuffer(command_buffer(), &begin_info) != VK_SUCCESS)
        throw std::runtime_error("Failed to begin recording command buffer");
}

void vulkan_command_buffer::end() {
    if (vkEndCommandBuffer(command_buffer()) != VK_SUCCESS) throw std::runtime_error("Failed to record command buffer");
}

void vulkan_command_buffer::begin_render_pass(const graphics_render_pass& render_pass) {
    const auto& native_render_pass = (const vulkan_render_pass&) render_pass;
    const auto& native_swapchain = (const vulkan_swapchain&) render_pass.swapchain();

    VkFramebuffer framebuffer = native_swapchain.current_framebuffer(native_render_pass.render_pass());
    VkRenderPassBeginInfo render_pass_info = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = native_render_pass.render_pass(),
        .framebuffer = framebuffer,
        .renderArea =
            {
                .offset = {0, 0},
                .extent = native_swapchain.extent(),
            },
        .clearValueCount = native_render_pass.vk_clear_values_count(),
        .pClearValues = native_render_pass.vk_clear_values(),
    };
    vkCmdBeginRenderPass(command_buffer(), &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport = {
        .x = 0.0f,
        .y = (float) native_swapchain.extent().height,
        .width = (float) native_swapchain.extent().width,
        .height = -(float) native_swapchain.extent().height,
        .minDepth = 0.0f,
        .maxDepth = 1.0f,
    };
    vkCmdSetViewport(command_buffer(), 0, 1, &viewport);

    VkRect2D scissor = {
        .offset = {0, 0},
        .extent = native_swapchain.extent(),
    };
    vkCmdSetScissor(command_buffer(), 0, 1, &scissor);
}

void vulkan_command_buffer::end_render_pass() {
    vkCmdEndRenderPass(command_buffer());
}

void vulkan_command_buffer::bind_pipeline(const graphics_pipeline& pipeline) {
    const auto& native_pipeline = (const vulkan_pipeline&) pipeline;
    vkCmdBindPipeline(command_buffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, native_pipeline.pipeline());
}

void vulkan_command_buffer::bind_vertex_buffer(const graphics_buffer& buffer, uint32_t offset, int index) {
    const auto& native_buffer = (const vulkan_buffer&) buffer;
    VkBuffer vertex_buffers[] = {native_buffer.buffer()};
    VkDeviceSize offsets[] = {offset};
    vkCmdBindVertexBuffers(command_buffer(), index, 1, vertex_buffers, offsets);
}

void vulkan_command_buffer::bind_resource_set(const graphics_resource_set& resource_set) {

    const auto& native_resource_set = (const vulkan_resource_set&) resource_set;
    VkDescriptorSet sets[] = {native_resource_set.descriptor_set()};
    vkCmdBindDescriptorSets(command_buffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, native_resource_set.pipeline_layout(),
                            native_resource_set.ref()->backend_number, 1, sets, 0, nullptr);
}

void vulkan_command_buffer::draw(uint32_t vertex_start, uint32_t vertex_count, uint32_t instance_start,
                                 uint32_t instance_count) {
    vkCmdDraw(command_buffer(), vertex_count, instance_count, vertex_start, instance_start);
}

void vulkan_command_buffer::draw_indexed(const graphics_buffer& index_buffer, uint32_t index_offset, index_type type,
                                         uint32_t index_start, uint32_t index_count, uint32_t vertex_offset,
                                         uint32_t instance_start, uint32_t instance_count) {
    const auto& native_buffer = (const vulkan_buffer&) index_buffer;
    VkIndexType index_type;
    switch (type) {
        case index_type::uint_16:
            index_type = VK_INDEX_TYPE_UINT16;
            break;
        case index_type::uint_32:
            index_type = VK_INDEX_TYPE_UINT32;
            break;
        default:
            throw std::runtime_error("Unsupported index type");
    }

    vkCmdBindIndexBuffer(command_buffer(), native_buffer.buffer(), index_offset, index_type);
    vkCmdDrawIndexed(command_buffer(), index_count, instance_count, index_start, (int32_t) vertex_offset,
                     instance_start);
}

#ifndef XGRAPHICS_VULKAN_PIPELINE_H
#define XGRAPHICS_VULKAN_PIPELINE_H

#include <result/result.h>
#include <vulkan/vulkan.h>
#include <xgraphics/interfaces/graphics_pipeline.h>

class vulkan_pipeline : public graphics_pipeline {
    VkDevice _device;
    VkPipeline _pipeline;

    vulkan_pipeline(const graphics_pipeline_init& init, VkDevice device, VkPipeline pipeline);

  public:
    ~vulkan_pipeline() override;

    static result::ptr<graphics_pipeline> create(const graphics_pipeline_init& init, VkDevice device);

    [[nodiscard]] VkPipeline pipeline() const;
};

#endif

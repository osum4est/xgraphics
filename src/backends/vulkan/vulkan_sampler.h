#ifndef XGRAPHICS_VULKAN_SAMPLER_H
#define XGRAPHICS_VULKAN_SAMPLER_H

#include <result/result.h>
#include <vulkan/vulkan.h>
#include <xgraphics/interfaces/graphics_sampler.h>

class vulkan_sampler : public graphics_sampler {
    VkDevice _device;
    VkSampler _sampler;

    vulkan_sampler(const graphics_sampler_init& init, VkDevice device, VkSampler sampler);

  public:
    ~vulkan_sampler() override;

    static result::ptr<graphics_sampler> create(const graphics_sampler_init& init, VkDevice device);

    [[nodiscard]] VkSampler sampler() const;

  private:
    static result::val<VkFilter> vk_filter(graphics_sampler_filter filter);
    static result::val<VkSamplerAddressMode> vk_address_mode(graphics_sampler_address_mode address_mode);
};

#endif

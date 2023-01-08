#ifndef XGRAPHICS_METAL_SAMPLER_H
#define XGRAPHICS_METAL_SAMPLER_H

#import <Metal/Metal.h>
#import <result/result.h>
#import <xgraphics/interfaces/graphics_sampler.h>

class metal_sampler : public graphics_sampler {
    id<MTLSamplerState> _sampler;

    metal_sampler(const graphics_sampler_init& init, id<MTLSamplerState> sampler);

  public:
    static result::ptr<graphics_sampler> create(const graphics_sampler_init& init, id<MTLDevice> device);

    [[nodiscard]] id<MTLSamplerState> sampler() const;

  private:
    static result::val<MTLSamplerMinMagFilter> mtl_filter(graphics_sampler_filter filter);
    static result::val<MTLSamplerAddressMode> mtl_address_mode(graphics_sampler_address_mode address_mode);
};

#endif

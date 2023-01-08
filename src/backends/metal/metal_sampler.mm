#import "metal_sampler.h"

metal_sampler::metal_sampler(const graphics_sampler_init& init, id<MTLSamplerState> sampler)
    : graphics_sampler(init), _sampler(sampler) { }

result::ptr<graphics_sampler> metal_sampler::create(const graphics_sampler_init& init, id<MTLDevice> device) {
    MTLSamplerDescriptor* descriptor = [[MTLSamplerDescriptor alloc] init];
    descriptor.sAddressMode = mtl_address_mode(init.address_mode.u).get();
    descriptor.tAddressMode = mtl_address_mode(init.address_mode.v).get();
    descriptor.rAddressMode = mtl_address_mode(init.address_mode.w).get();
    descriptor.minFilter = mtl_filter(init.min_filter).get();
    descriptor.magFilter = mtl_filter(init.mag_filter).get();
    descriptor.mipFilter = MTLSamplerMipFilterNotMipmapped;
    descriptor.normalizedCoordinates = YES;
    descriptor.lodMinClamp = 0.0;
    descriptor.lodMaxClamp = 0.0;
    descriptor.maxAnisotropy = init.enable_anisotropy ? init.max_anisotropy : 1.0f;
    descriptor.compareFunction = MTLCompareFunctionAlways;
    descriptor.supportArgumentBuffers = YES;

    id<MTLSamplerState> sampler = [device newSamplerStateWithDescriptor:descriptor];
    return result::ok(new metal_sampler(init, sampler));
}

id<MTLSamplerState> metal_sampler::sampler() const {
    return _sampler;
}

result::val<MTLSamplerMinMagFilter> metal_sampler::mtl_filter(graphics_sampler_filter filter) {
    switch (filter) {
        case graphics_sampler_filter::nearest:
            return result::ok(MTLSamplerMinMagFilterNearest);
        case graphics_sampler_filter::linear:
            return result::ok(MTLSamplerMinMagFilterLinear);
        default:
            return result::err("Unsupported filter");
    }
}

result::val<MTLSamplerAddressMode> metal_sampler::mtl_address_mode(graphics_sampler_address_mode address_mode) {
    switch (address_mode) {
        case graphics_sampler_address_mode::repeat:
            return result::ok(MTLSamplerAddressModeRepeat);
        case graphics_sampler_address_mode::mirror_repeat:
            return result::ok(MTLSamplerAddressModeMirrorRepeat);
        case graphics_sampler_address_mode::clamp_to_edge:
            return result::ok(MTLSamplerAddressModeClampToEdge);
        case graphics_sampler_address_mode::mirror_clamp_to_edge:
            return result::ok(MTLSamplerAddressModeMirrorClampToEdge);
        case graphics_sampler_address_mode::clamp_to_border:
            return result::ok(MTLSamplerAddressModeClampToBorderColor);
        default:
            return result::err("Unsupported address mode");
    }
}

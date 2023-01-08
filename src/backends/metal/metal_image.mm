#import "metal_image.h"

metal_image::metal_image(uint32_t width, uint32_t height, graphics_image_format format, id<MTLTexture> texture)
    : graphics_image(width, height, format), _texture(texture) { }

result::ptr<graphics_image> metal_image::create(uint32_t width, uint32_t height, graphics_image_format format,
                                                id<MTLDevice> device) {

    MTLPixelFormat pixel_format;
    switch (format) {
        case graphics_image_format::rgba_8_srgb:
            pixel_format = MTLPixelFormatRGBA8Unorm_sRGB;
            break;
        case graphics_image_format::rgba_8_unorm:
            pixel_format = MTLPixelFormatRGBA8Unorm;
            break;
        default:
            return result::err("Unsupported pixel format");
    }

    MTLTextureDescriptor* descriptor = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:pixel_format
                                                                                          width:width
                                                                                         height:height
                                                                                      mipmapped:NO];
    descriptor.usage = MTLTextureUsageShaderRead;
    id<MTLTexture> texture = [device newTextureWithDescriptor:descriptor];
    return result::ok(new metal_image(width, height, format, texture));
}

id<MTLTexture> metal_image::texture() const {
    return _texture;
}

void metal_image::write(const void* data, uint32_t size) {
    int channels;
    switch (format()) {
        case graphics_image_format::rgba_8_srgb:
        case graphics_image_format::rgba_8_unorm:
            channels = 4;
            break;
        default:
            return;
    }

    MTLRegion region = MTLRegionMake2D(0, 0, width(), height());
    [_texture replaceRegion:region mipmapLevel:0 withBytes:data bytesPerRow:width() * channels];
}

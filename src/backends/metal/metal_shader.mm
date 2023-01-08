#import "metal_shader.h"

#import <vector>

metal_shader::metal_shader(std::unique_ptr<shader_binary> binary, id<MTLDevice> device, id<MTLFunction> function)
    : graphics_shader(std::move(binary)), _device(device), _function(function) { }

result::ptr<graphics_shader> metal_shader::create(std::unique_ptr<shader_binary> binary, id<MTLDevice> device) {
    auto source = [[NSString alloc] initWithBytes:binary->data().data()
                                           length:binary->data().size()
                                         encoding:NSUTF8StringEncoding];

    NSError* error = nil;
    id<MTLLibrary> library = [device newLibraryWithSource:source options:nil error:&error];
    if (!library)
        return result::err(std::string("Failed to create shader library: ") + error.localizedDescription.UTF8String);

    auto entry_point = [NSString stringWithUTF8String:binary->info().entry_point.c_str()];
    id<MTLFunction> function = [library newFunctionWithName:entry_point];
    if (!function) return result::err("Failed to create shader function");

    return result::ok(new metal_shader(std::move(binary), device, function));
}

id<MTLFunction> metal_shader::function() const {
    return _function;
}

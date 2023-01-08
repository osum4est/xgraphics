#ifndef XGRAPHICS_METAL_INSTANCE_H
#define XGRAPHICS_METAL_INSTANCE_H

#import <MetalKit/MetalKit.h>
#import <xgraphics/xgraphics.h>

class metal_instance : public graphics_instance {
    CAMetalLayer* _layer;

  public:
    explicit metal_instance(const graphics_config& config, CAMetalLayer* layer);

    static result::ptr<graphics_instance> create(const graphics_config& config);

    std::vector<std::unique_ptr<graphics_device_def>> list_devices() override;
    result::ptr<graphics_device> create_device(std::unique_ptr<graphics_device_def> def) override;
};

#endif
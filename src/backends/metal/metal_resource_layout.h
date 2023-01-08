#ifndef XGRAPHICS_METAL_RESOURCE_LAYOUT_H
#define XGRAPHICS_METAL_RESOURCE_LAYOUT_H

#import <xgraphics/interfaces/graphics_resource_layout.h>

class metal_resource_layout : public graphics_resource_layout {
  protected:
    explicit metal_resource_layout(const std::vector<const graphics_shader*>& stages);

  public:
    static result::ptr<graphics_resource_layout> create(const std::vector<const graphics_shader*>& stages);
};

#endif

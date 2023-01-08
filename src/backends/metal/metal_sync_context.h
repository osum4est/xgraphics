#ifndef XGRAPHICS_METAL_SYNC_CONTEXT_H
#define XGRAPHICS_METAL_SYNC_CONTEXT_H

#import <dispatch/dispatch.h>
#import <result/result.h>
#import <xgraphics/graphics_config.h>

class metal_sync_context {
    dispatch_semaphore_t _semaphore;
    uint32_t _current_frame = 0;
    uint32_t _frames_in_flight;

    explicit metal_sync_context(dispatch_semaphore_t semaphore, uint32_t frames_in_flight);

  public:
    metal_sync_context(const metal_sync_context&) = delete;

    static result::ptr<metal_sync_context> create(graphics_config config);

    void set_current_frame(int current_frame);

    [[nodiscard]] dispatch_semaphore_t semaphore() const;
    [[nodiscard]] uint32_t current_frame() const;
    [[nodiscard]] uint32_t frames_in_flight() const;
};

#endif

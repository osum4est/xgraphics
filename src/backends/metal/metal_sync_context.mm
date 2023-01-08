#include "metal_sync_context.h"

metal_sync_context::metal_sync_context(dispatch_semaphore_t semaphore, uint32_t frames_in_flight)
    : _semaphore(semaphore), _frames_in_flight(frames_in_flight) { }

result::ptr<metal_sync_context> metal_sync_context::create(graphics_config config) {
    auto frames_in_flight = config.frames_in_flight;
    return result::ok(
        new metal_sync_context(dispatch_semaphore_create(frames_in_flight), frames_in_flight));
}

void metal_sync_context::set_current_frame(int current_frame) {
    _current_frame = current_frame;
}

dispatch_semaphore_t metal_sync_context::semaphore() const {
    return _semaphore;
}

uint32_t metal_sync_context::current_frame() const {
    return _current_frame;
}

uint32_t metal_sync_context::frames_in_flight() const {
    return _frames_in_flight;
}

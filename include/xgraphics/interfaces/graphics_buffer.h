#ifndef WPEX_GRAPHICS_BUFFER_H
#define WPEX_GRAPHICS_BUFFER_H

#include <cstdint>

struct buffer_usage {
    enum buffer_usage_bits {
        vertex = 1 << 0,
        index = 1 << 1,
    };
};

typedef uint32_t buffer_usage_flags;

class graphics_buffer {
    buffer_usage_flags _usage;
    uint32_t _size;

  protected:
    explicit graphics_buffer(buffer_usage_flags usage, uint32_t size);

  public:
    graphics_buffer(const graphics_buffer&) = delete;
    virtual ~graphics_buffer() = default;

    [[nodiscard]] buffer_usage_flags usage() const;
    [[nodiscard]] uint32_t size() const;

    virtual void write(const void* data, uint32_t size) = 0;
};

#endif

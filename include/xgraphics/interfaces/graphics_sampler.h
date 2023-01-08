#ifndef WPEX_GRAPHICS_SAMPLER_H
#define WPEX_GRAPHICS_SAMPLER_H

enum class graphics_sampler_filter {
    nearest,
    linear,
};

enum class graphics_sampler_address_mode {
    repeat,
    mirror_repeat,
    clamp_to_edge,
    mirror_clamp_to_edge,
    clamp_to_border,
};

struct graphics_sampler_init {
    graphics_sampler_filter min_filter = graphics_sampler_filter::linear;
    graphics_sampler_filter mag_filter = graphics_sampler_filter::linear;
    struct {
        graphics_sampler_address_mode u;
        graphics_sampler_address_mode v;
        graphics_sampler_address_mode w;
    } address_mode;
    bool enable_anisotropy = true;
    float max_anisotropy = 1.0f;
};

class graphics_sampler {
    graphics_sampler_init _init;

  protected:
    explicit graphics_sampler(const graphics_sampler_init& init);

  public:
    graphics_sampler(const graphics_sampler&) = delete;
    virtual ~graphics_sampler() = default;
};

#endif

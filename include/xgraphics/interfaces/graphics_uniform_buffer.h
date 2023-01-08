#ifndef WPEX_GRAPHICS_UNIFORM_BUFFER_H
#define WPEX_GRAPHICS_UNIFORM_BUFFER_H

#include <result/result.h>
#include <xgraphics/shaders/shader_data.h>

typedef const shader_struct_member* uniform_member_ref;

class graphics_uniform_buffer {
    shader_variable_type _type;
    uint32_t _size;

  protected:
    explicit graphics_uniform_buffer(const shader_variable_type& type, uint32_t size);

    inline virtual void set_data(const shader_variable_type& type, uint32_t offset, uint32_t size,
                                 const void* data) = 0;

  public:
    virtual ~graphics_uniform_buffer() = default;

    void set_mat_4(uniform_member_ref ref, const float* value);

    [[nodiscard]] const shader_variable_type& type() const;
    [[nodiscard]] uint32_t size() const;

    [[nodiscard]] result::val<uniform_member_ref> member_by_name(const std::string& name) const;
};

#endif

#include "xgraphics/interfaces/graphics_uniform_buffer.h"

#include <queue>

graphics_uniform_buffer::graphics_uniform_buffer(const shader_variable_type& type, uint32_t size)
    : _type(type), _size(size) { }

const shader_variable_type& graphics_uniform_buffer::type() const {
    return _type;
}

uint32_t graphics_uniform_buffer::size() const {
    return _size;
}

void graphics_uniform_buffer::set_mat_4(uniform_member_ref ref, const float* value) {
    set_data(ref->type, ref->offset, sizeof(float) * 16, value);
}

result::val<uniform_member_ref> graphics_uniform_buffer::member_by_name(const std::string& name) const {
    std::queue<std::string> parts;
    size_t start = 0;
    while (start < name.size()) {
        size_t end = name.find('.', start);
        if (end == std::string::npos) {
            parts.push(name.substr(start));
            break;
        }
        parts.push(name.substr(start, end - start));
        start = end + 1;
    }

    const shader_variable_type* current_type = &_type;
    const shader_struct_member* member = nullptr;
    while (!parts.empty()) {
        const auto& part = parts.front();
        member = nullptr;

        if (current_type->base_type != shader_variable_base_type::structure)
            return result::err("Variable is not a struct");
        for (const auto& struct_member : std::get<shader_struct_variable>(current_type->data).members) {
            if (struct_member.name == part) {
                member = &struct_member;
                current_type = &struct_member.type;
                break;
            }
        }

        if (!member) return result::err("Member not found");
        parts.pop();
    }

    return result::ok(member);
}

#include "native/schema_system.hpp"
#include "container/span.h"
#include "functional/vfunc.h"

#include <assert.h>

#include <algorithm>

namespace fd::native
{
schema_system_type_scope* schema_system::find_type_scope_for(char const* module_name)
{
    using fn_t = schema_system_type_scope* (schema_system::*)(char const*, void*);
    return vfunc<fn_t>{13, this}(module_name, nullptr);
}

schema_class_info* schema_system_type_scope::find_declared_class(char const* name)
{
#ifdef _WIN32
    using fn_t = void (schema_system_type_scope::*)(schema_class_info**, char const*);
#elif defined(__linux__)
    using fn_t = schema_class_info* (schema_system_type_scope::*)(char const*);
#endif

    schema_class_info* binding;
    vfunc<fn_t> vfunc{2, this};

#ifdef _WIN32
    vfunc(&binding, name);
#elif defined(__linux__)
    binding = vfunc(name);
#endif
    return binding;
}

bool schema_class_info::inherits_from(schema_class_info* other) const
{
    assert(base_classes != nullptr);
    assert(other != nullptr);
    if (this == other)
        return true;
    return std::any_of(base_classes, base_classes + base_classes_count, [=](schema_base_class_info_data const& data) {
        return data.target->inherits_from(other);
    });
}

int32_t schema_class_info::field_offset(char const* name, size_t const name_length) const
{
    auto const last_field   = fields + fields_count;
    auto const target_field = std::find_if(fields, last_field, [=](schema_class_field_data const& data) {
        return data.name[name_length] == '\0' && memcmp(data.name, name, name_length) == 0;
    });
    assert(target_field != last_field);
    return target_field->single_inheritance_offset;
}
}
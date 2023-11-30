#include "tier1/container/span.h"
#include "tier1/functional/vfunc.h"
#include "tier2/native/schema_system.hpp"

#include <assert.h>

namespace FD_TIER2(native, cs2)
{
schema_system_type_scope* schema_system::find_type_scope_for(char const* module_name)
{
    using fn_t = schema_system_type_scope* (schema_system::*)(char const*, void*);
    return vfunc<fn_t>{13, this}(module_name, nullptr);
}

schema_class_info* schema_system_type_scope::find_declared_class(char const* name)
{
    using fn_t = void (schema_system_type_scope::*)(schema_class_info**, char const*);
    schema_class_info* binding;
    vfunc<fn_t>{2, this}(&binding, name);
    return binding;
}

bool schema_class_info::inherits_from(schema_class_info* other) const
{
    assert(base_classes != nullptr);
    assert(other != nullptr);
    if (this == other)
        return true;

    for (auto it = base_classes, last_class = base_classes + base_classes_count; it != last_class; ++it)
    {
        if (it->target->inherits_from(other))
            return true;
    }

    return false;
}

int32_t schema_class_info::field_offset(char const* name, size_t const name_length) const
{
    for (auto it = fields, last_field = fields + fields_count; it != last_field; ++it)
    {
        if (it->name[name_length] != '\0')
            continue;
        if (memcmp(it->name, name, name_length) != 0)
            continue;
        return it->single_inheritance_offset;
    }

    return -1;
}
}
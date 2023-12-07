#pragma once

#include "functional/vfunc.h"

#include <assert.h>

namespace fd::native::inline cs2
{
class schema_class_info;

class schema_system_type_scope
{
  public:
    schema_class_info* find_declared_class(char const* name)
    {
        using fn_t = void (schema_system_type_scope::*)(schema_class_info**, char const*);
        schema_class_info* binding;
        vfunc<fn_t>{2, this}(&binding, name);
        return binding;
    }
};

struct schema_base_class_info_data
{
    int32_t offset;
    schema_class_info* target;
};

// class schema_base_class_info : public schema_base_class_info_data
// {
//   public:
// };

struct schema_class_field_data
{
    char const* name;
    char pad0[0x8];
    int32_t single_inheritance_offset;
    char pad1[0xC];
};

// class schema_class_field : public schema_class_field_data
// {
//   public:
// };

struct schema_class_info_data
{
    schema_class_info_data* self;              // 0x00
    char const* name;                          // 0x08
    char const* description;                   // 0x10
    int32_t size_of;                           // 0x18
    uint8_t fields_count;                      // 0x1C
    char pad1[0x5];                            // 0x1D
    uint8_t align_of;                          // 0x22
    uint8_t base_classes_count;                // 0x23
    char pad2[0x4];                            // 0x24
    schema_class_field_data* fields;           // 0x28
    char pad3[0x8];                            // 0x30
    schema_base_class_info_data* base_classes; // 0x38
    char pad4[0x28];                           // 0x40
};

class schema_class_info : public schema_class_info_data
{
  public:
    // Full implementation here:
    // https://github.com/neverlosecc/source2gen/blob/main/include/sdk/interfaces/schemasystem/schema.h#L409C1

    bool inherits_from(schema_class_info* other) const
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

    int32_t field_offset(char const* name, size_t const name_length) const
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
};

// SchemaSystem_001
class schema_system
{
  public:
    schema_system_type_scope* find_type_scope_for(char const* module_name)
    {
        using fn_t = schema_system_type_scope* (schema_system::*)(char const*, void*);
        return vfunc<fn_t>{13, this}(module_name, nullptr);
    }
};
}
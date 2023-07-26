#pragma once

#include <cstdint>

namespace fd
{
struct native_recv_table
{
    enum class prop_type
    {
        int32 = 0,
        floating,
        vector3d,
        vector2d,
        string,
        array,
        data_table,
        int64
    };

    struct prop
    {
        char const *name;
        prop_type type;
        int flags;
        int string_buffer_size;
        int inside_array;
        void const *extra_data;
        prop *array_prop;
        void *array_length_proxy;
        void *proxy;
        void *data_table_proxy;
        native_recv_table *data_table;
        int offset;
        int element_stride;
        int elements_count;
        char const *parent_array_name;
    };

    using value_type = prop; // for compatibility

    prop *props;
    uint32_t props_count;
    void *decoder;
    char const *name;
    bool initialized;
    bool in_main_list;
};
} // namespace fd
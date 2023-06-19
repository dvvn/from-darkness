#pragma once

#include <cstdint>

namespace fd::valve
{
enum class recv_prop_type
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

struct recv_prop
{
    char const *name;
    recv_prop_type type;
    int flags;
    int string_buffer_size;
    int inside_array;
    void const *extra_data;
    recv_prop *array_prop;
    void *array_length_proxy;
    void *proxy;
    void *data_table_proxy;
    struct recv_table *data_table;
    int offset;
    int element_stride;
    int elements_count;
    char const *parent_array_name;
};

struct recv_table
{
    recv_prop *props;
    uint32_t props_count;
    void *decoder;
    char const *name;
    bool initialized;
    bool in_main_list;
};
} // namespace fd::valve
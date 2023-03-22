#pragma once

#include <span>

namespace fd::valve
{
enum recv_prop_type
{
    DPT_Int = 0,
    DPT_Float,
    DPT_Vector,
    DPT_VectorXY,
    DPT_String,
    DPT_Array,
    DPT_DataTable,
    DPT_Int64
};

struct recv_table;

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
    recv_table *data_table;
    int offset;
    int element_stride;
    int elements_count;
    char const *parent_array_name;
};

struct recv_table
{
    std::span<recv_prop> props;
    void *decoder;
    char const *name;
    bool initialized;
    bool in_main_list;
};
} // namespace fd::valve
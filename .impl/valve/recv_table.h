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

struct recv_prop;

struct recv_proxy_data
{
    struct value_type
    {
        union
        {
            float    m_Float;
            long     m_Int;
            char*    String;
            void*    m_data;
            float    m_Vector[3];
            uint64_t m_Int64;
        };

        recv_prop_type m_Type;
    };

    const recv_prop* prop;      // The property it's receiving.
    value_type       value;     // The value given to you to store.
    int              element;   // Which array element you're getting.
    int              object_id; // The object being referred to.
};

struct recv_table;

struct recv_prop
{
    // ArrayLengthRecvProxies are optionally used to Get the length of the
    // incoming array when it changes.
    using array_length_proxy_fn = void (*)(void* struct_ptr, int object_id, int current_array_length);

    // struct_ptr = the base structure of the datatable this variable is in (like base_entity)
    // out    = the variable that this this proxy represents (like base_entity::m_SomeValue).
    //
    // Convert the network-standard-type value in m_Value into your own format in struct_ptr/out.
    using proxy_fn = void (*)(const recv_proxy_data* data, void* struct_ptr, void* out);

    // NOTE: DataTable receive proxies work differently than the other proxies.
    // data points at the object + the recv table's offset.
    // out should be Set to the location of the object to unpack the data table into.
    // If the parent object just contains the child object, the default proxy just does *out = data.
    // If the parent object points at the child object, you need to dereference the pointer here.
    // NOTE: don't ever return null from a DataTable receive proxy function. Bad things will happen.
    using data_table_proxy_fn = void (*)(const recv_prop* pProp, void** out, void* data, int object_id);

    const char*           name;
    recv_prop_type        type;
    int                   flags;
    int                   string_buffer_size;
    int                   inside_array;
    const void*           extra_data;
    recv_prop*            array_prop;
    array_length_proxy_fn array_length_proxy;
    proxy_fn     proxy;
    data_table_proxy_fn   data_table_proxy;
    recv_table*           data_table;
    int                   offset;
    int                   element_stride;
    int                   elements_count;
    const char*           parent_array_name;
};

struct recv_table
{
    std::span<recv_prop> props;
    void*                decoder;
    const char*          name;
    bool                 initialized;
    bool                 in_main_list;
};
} // namespace fd::valve
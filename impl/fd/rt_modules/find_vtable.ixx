module;

#include <fd/rt_modules/winapi_fwd.h>

export module fd.rt_modules:find_vtable;
export import fd.string;

void* find_vtable_class(LDR_DATA_TABLE_ENTRY* const ldr_entry, const fd::string_view name, const bool notify = true);
void* find_vtable_struct(LDR_DATA_TABLE_ENTRY* const ldr_entry, const fd::string_view name, const bool notify = true);
void* find_vtable(LDR_DATA_TABLE_ENTRY* const ldr_entry, const fd::string_view name, const bool notify = true);
void* find_vtable(LDR_DATA_TABLE_ENTRY* const ldr_entry, decltype(typeid(int)) info, const bool notify = true);

template <class T>
constexpr auto simple_type_name()
{
    return __FUNCSIG__;
}

#ifndef __cpp_lib_string_contains
#define contains(x) find(x) != static_cast<size_t>(-1)
#endif

template <class T>
T* find_vtable(LDR_DATA_TABLE_ENTRY* const ldr_entry, const bool notify = true)
{
    constexpr fd::string_view raw_name = simple_type_name<T>();
    // full name with 'class' or 'struct' prefix, namespace and templates
    constexpr fd::string_view name(raw_name.data() + raw_name.find('<') + 1, raw_name.data() + raw_name.rfind('>'));

    void* ptr;
    if constexpr (name.contains('>') || name.contains(':')) // namespaces and templates currently unsupported
        ptr = find_vtable(ldr_entry, typeid(T), notify);
    else
        ptr = find_vtable(ldr_entry, name, notify);
    return static_cast<T*>(ptr);
}

export namespace fd
{
    using ::find_vtable;
    using ::find_vtable_class;
    using ::find_vtable_struct;
} // namespace fd

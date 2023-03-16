#pragma once

#include <cassert>
#include <string_view>

namespace fd
{
size_t get_netvar_offset(std::string_view table, std::string_view name);

template <class T>
void init_netvars();

#ifdef NETVAR_CLASS
namespace valve
{
static void init_current_netvars();
}

template <>
void init_netvars<valve::NETVAR_CLASS>()
{
    valve::init_current_netvars();
}

#if 0
template <auto T>
static constexpr size_t extra_netvar_offset = 0;

#define SET_EXTRA_NETVAR_OFFSET(_NAME_, _OFFSET_) \
    template <>                                   \
    static constexpr size_t extra_netvar_offset<#_NAME_> = _OFFSET_;
#endif

inline void set_netvar_offset(size_t &offset, std::string_view table, std::string_view name = "off")
{
    assert(offset == -1);
    offset = get_netvar_offset(table, name) /*+ extra_netvar_offset<Name>*/;
}

template <typename T>
static decltype(auto) get_netvar(void *this_ptr, size_t offset)
{
    assert(offset != -1);
    auto netvar = reinterpret_cast<uintptr_t>(this_ptr) + offset;
    if constexpr (std::is_pointer_v<T>)
        return reinterpret_cast<T>(netvar);
    else
        return *reinterpret_cast<std::remove_reference_t<T> *>(netvar);
}
#endif

}
#pragma once

#include "pattern.h"
#include "library_info/native.h"
#include "library_info/tag.h"

namespace fd
{
template <library_tag Tag>
struct native_return_address_gadget;

struct default_return_address_gadget_pattern
{
    static constexpr auto value = "FF 23"_pat;
};

template <library_tag Tag>
struct return_address_gadget_pattern : default_return_address_gadget_pattern
{
};

template <library_tag Tag>
struct native_library : native_library_info
{
    native_library()
        : native_library_info(Tag)
    {
        native_return_address_gadget<Tag>::address = reinterpret_cast<uintptr_t>(
            this->pattern(return_address_gadget_pattern<Tag>::value));
    }

    operator system_library<Tag>() const
    {
        return static_cast<system_library_info>(*this);
    }
};

struct native_sources
{
#define NATIVE_SOURCE(_NAME_) native_library<#_NAME_> _NAME_;

    NATIVE_SOURCE(client);
    NATIVE_SOURCE(engine);
    NATIVE_SOURCE(vguimatsurface);
    NATIVE_SOURCE(shaderapidx9);
    
#undef NATIVE_SOURCE
};
} // namespace fd
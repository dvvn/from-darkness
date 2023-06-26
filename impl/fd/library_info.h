#pragma once

#include "call_type.h"
#include "core.h"

#include "library_info/native.h"
#include "library_info/system.h"
#include "library_info/tag.h"
#include "tool/string.h"

namespace fd
{
template <library_tag Tag>
struct system_library : system_library_info
{
    system_library()
        : system_library_info(Tag)
    {
    }

    system_library(system_library_info info)
        : system_library_info(info)
    {
    }
};

template <library_tag Tag>
struct native_return_address_gadget;

template <library_tag Tag>
struct native_library : native_library_info
{
    native_library()
        : native_library_info(Tag)
    {
        native_return_address_gadget<Tag>::address = reinterpret_cast<uintptr_t>(this->pattern("FF 23"));
    }

    operator system_library<Tag>() const
    {
        return static_cast<system_library_info>(*this);
    }
};
}
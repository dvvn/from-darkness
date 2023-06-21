#pragma once

#include "library_info.h"

#undef interface

namespace fd
{
struct valve_library : system_library
{
    using system_library::system_library;

    valve_library(system_library other);

    void *interface(string_view name) const;
};

// template <named_arg Name>
// struct named_valve_library : valve_library
//{
//     named_valve_library()
//     {
//         wchar_t buff[Name.length()];
//         std::copy(Name.begin(), Name.end(), buff);
//         std::construct_at<system_library>(this, buff, Name.length());
//     }
// };

} // namespace fd
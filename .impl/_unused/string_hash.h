#pragma once

#include <fd/chars_cache.h>
#include <fd/hash.h>

namespace fd
{
    template <typename C>
    struct hash<basic_string_view<C>>
    {
        constexpr size_t operator()(const basic_string_view<C> str) const
        {
            return hash_bytes(str.data(), str.size());
        }
    };

    template <typename C>
    struct hash<basic_string<C>> : hash<basic_string_view<C>>
    {
        constexpr size_t operator()(const basic_string<C>& str) const
        {
            return hash_bytes(str.data(), str.size());
        }
    };

    inline namespace literals
    {
        template <chars_cache Cache>
        consteval size_t operator"" _hash()
        {
            return hash_bytes(Cache.data(), Cache.size());
        }
    } // namespace literals

} // namespace fd

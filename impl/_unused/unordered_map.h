#pragma once

#if __has_include(<robin_hood.h>)
#include <robin_hood.h>
#else
#include <fd/hash.h>
#include <unordered_map>
#endif

namespace fd
{
    template <typename Key, typename Value>
    using unordered_map =
#ifdef ROBIN_HOOD
        robin_hood::unordered_map<Key, Value>
#else
        std::unordered_map<Key, Value, hash<Key>>
#endif
        ;

}
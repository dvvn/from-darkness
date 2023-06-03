#pragma once

#include "callback.h"
#include "core.h"

namespace fd
{
using find_filter = basic_callback<bool>;
template <typename Arg, typename Fn>
using find_callback = callback<bool, callback_function_proxy<Arg, std::reference_wrapper<Fn>>>;

#define MAKE_FILTER(_RET_) find_callback<_RET_, Fn>(filter).base()

void *find_pattern(void *begin, void *end, _const<char*>pattern, size_t pattern_length, find_filter *filter = nullptr);

template <typename Fn>
void *find_pattern(void *begin, void *end, _const<char*>pattern, size_t pattern_length, Fn filter)
{
    return find_pattern(begin, end, pattern, pattern_length, MAKE_FILTER(void *));
}

uintptr_t find_xref(void *begin, void *end, uintptr_t &address, find_filter *filter = nullptr);

template <typename Fn>
uintptr_t find_xref(void *begin, void *end, uintptr_t &address, Fn filter)
{
    return find_xref(begin, end, address, MAKE_FILTER(uintptr_t &));
}

void *find_bytes(void *begin, void *end, _const<void*>bytes, size_t length, find_filter *filter = nullptr);

template <typename Fn>
void *find_bytes(void *begin, void *end, _const<void*>bytes, size_t length, Fn filter)
{
    return find_bytes(begin, end, bytes, length, MAKE_FILTER(void *));
}

#undef MAKE_FILTER
} // namespace fd
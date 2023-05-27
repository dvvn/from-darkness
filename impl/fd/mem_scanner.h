#pragma once

#include "callback.h"

namespace fd
{
using find_filter = basic_callback<bool>;
template <typename Arg, typename Fn>
using find_callback            = callback<bool, callback_function_proxy<Arg, std::reference_wrapper<Fn>>>;
using find_callback_stop_token = callback_stop_token;

void *find_pattern(void *begin, void *end, char const *pattern, size_t pattern_length, find_filter *filter = nullptr);

template <typename Fn>
void *find_pattern(void *begin, void *end, char const *pattern, size_t pattern_length, Fn filter)
{
    return find_pattern(begin, end, pattern, pattern_length, find_callback<void *, Fn>(filter).base());
}

uintptr_t find_xref(void *begin, void *end, uintptr_t &address, find_filter *filter = nullptr);

template <typename Fn>
uintptr_t find_xref(void *begin, void *end, uintptr_t &address, Fn filter)
{
    return find_xref(begin, end, address, find_callback<uintptr_t &, Fn>(filter).base());
}

void *find_bytes(void *begin, void *end, void *bytes, size_t length, find_filter *filter = nullptr);

template <typename Fn>
void *find_bytes(void *begin, void *end, void *bytes, size_t length, Fn filter)
{
    return find_bytes(begin, end, bytes, length, find_callback<void *, Fn>(filter).base());
}
} // namespace fd
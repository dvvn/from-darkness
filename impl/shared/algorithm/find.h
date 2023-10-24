#pragma once
#include "memory/basic_pattern.h"

#include <boost/hana/front.hpp>
#include <boost/hana/unpack.hpp>

#include <algorithm>
#include <cassert>

namespace fd
{
namespace detail
{
template <typename Callback, typename V>
class find_callback_invoker
{
    static constexpr bool invocable_arg = std::invocable<Callback, V>;
    static constexpr bool invocable_raw = std::invocable<Callback>;

  public:
    static constexpr bool invocable = invocable_arg || invocable_raw;

    static auto invoke(Callback& callback, V found)
    {
        if constexpr (invocable_arg)
            return std::invoke(callback, found);
        else if constexpr (invocable_raw)
            return std::invoke(callback);
        else
            return true;
    }
};

template <typename V, typename Callback>
V* find_one_byte(V* first, V* last, V first_value, Callback callback)
{
    using callback_invoker = find_callback_invoker<Callback, V*>;
    if constexpr (callback_invoker::invocable)
    {
        for (;;)
        {
            auto const found = std::find(first, last, first_value);
            if (found == last)
                return nullptr;
            if (callback_invoker::invoke(callback, found))
                return found;
            first = found;
        }
    }
    else
    {
        auto const found = std::find(first, (last), first_value);
        return found == last ? nullptr : found;
    }
}
} // namespace detail

template <typename Callback = uint8_t>
void* find(void* begin, void const* end, void const* from, void const* to, Callback callback = {})
{
    auto b_begin     = static_cast<uint8_t*>(begin);
    auto const b_end = static_cast<uint8_t*>(const_cast<void*>(end));

    auto const b_from     = static_cast<uint8_t const*>(from);
    auto const b_to       = static_cast<uint8_t const*>((to));
    auto const first_from = static_cast<uint8_t const*>(from)[0];

    auto const target_length = std::distance(b_from, b_to);
    if (target_length == 1)
    {
        return detail::find_one_byte(b_begin, b_end, first_from, std::ref(callback));
    }
    // ReSharper disable once CppRedundantElseKeywordInsideCompoundStatement
    else
    {
        using callback_invoker = detail::find_callback_invoker<Callback, uint8_t*>;

        auto const b_end_safe = b_end - target_length;
        for (;;)
        {
            auto const first_found = std::find(b_begin, b_end_safe, first_from);
            if (first_found == b_end_safe)
                break;
            if (!std::equal(b_from, b_to, b_begin))
                ++b_begin;
            else
            {
                if (callback_invoker::invoke(callback, first_found))
                    return first_found;
                b_begin = first_found + target_length;
            }
        }
    }
    return nullptr;
}

template <detail::pattern_size_type BytesCount>
class pattern_segment;
template <detail::pattern_size_type... SegmentsBytesCount>
struct pattern;

template <detail::pattern_size_type BytesCount, typename... Next>
bool equal(uint8_t const* mem, pattern_segment<BytesCount> const& segment, Next const&... next)
{
    auto const ok = segment.equal(mem);

    if constexpr (sizeof...(Next) == 0)
        return ok;
    else
        return ok && equal(mem + (segment.abs_size()), next...);
}

template <detail::pattern_size_type... SegmentsBytesCount>
bool equal(uint8_t const* first, uint8_t const* last, pattern<SegmentsBytesCount...> const& pat)
{
    assert(std::distance(first, last) <= pat.size());
    return boost::hana::unpack(pat.bytes, [=](auto&... segment) -> bool {
        return equal(first, segment...);
    });
}

template <typename Callback = uint8_t, detail::pattern_size_type... SegmentsBytesCount>
void* find(uint8_t* first, uint8_t const* last, pattern<SegmentsBytesCount...> const& pat, Callback callback = {})
{
    auto const first_pattern_byte = *boost::hana::front(pat.bytes).begin();

    if constexpr (sizeof...(SegmentsBytesCount) == 1)
    {
        return detail::find_one_byte(first, const_cast<uint8_t*>(last), first_pattern_byte, std::ref(callback));
    }
    else
    {
        using callback_invoker = detail::find_callback_invoker<Callback, uint8_t*>;

        auto const pat_size  = pat.size();
        auto const safe_last = const_cast<uint8_t*>(last) - pat_size;

        for (;;)
        {
            auto const first_found = std::find(first, safe_last, first_pattern_byte);
            if (first_found == safe_last)
                break;

            if (!equal(first_found, safe_last, pat))
                ++first;
            else
            {
                if (callback_invoker::invoke(callback, first_found))
                    return first_found;
                first = first_found + pat_size;
            }
        }
    }
    return nullptr;
}

template <typename Callback = uint8_t, detail::pattern_size_type... SegmentsBytesCount>
void* find(void* first, void const* last, pattern<SegmentsBytesCount...> const& pat, Callback callback = {})
{
    return find(static_cast<uint8_t*>(first), static_cast<uint8_t const*>(last), pat, std::ref(callback));
}

template <bool Owned>
class xref;

template <typename Callback = uint8_t, bool Owned>
void* find(uint8_t* first, uint8_t const* last, xref<Owned> const& xr, Callback callback = {})
{
    auto ptr = xr.get();
    return find(first, last, ptr, ptr + 1, std::ref(callback));
}

template <typename Callback = uint8_t, bool Owned>
void* find(void* first, void const* last, xref<Owned> const& xr, Callback callback = {})
{
    return find(static_cast<uint8_t*>(first), static_cast<uint8_t const*>(last), xr, std::ref(callback));
}
}
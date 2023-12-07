#pragma once
#include "pattern/impl.h"
#include "library_info.h"

namespace fd
{
template <typename Fn>
void* library_info::basic_pattern_getter::find_in_section(Fn fn) const
{
    auto const nt       = linfo_->nt_header();
    auto const img_base = safe_cast<uint8_t>(linfo_->image_base());

    auto first_section      = IMAGE_FIRST_SECTION(nt);
    auto const last_section = first_section + nt->FileHeader.NumberOfSections;

    for (; first_section != last_section; ++first_section)
    {
        auto first      = img_base + first_section->VirtualAddress;
        auto const last = first + first_section->SizeOfRawData;

        auto const found = fn(first, last);
        if (found == last)
            continue;
        return found;
    }

    return nullptr;
}

template <typename Fn>
void* library_info::basic_pattern_getter::find_anywhere(Fn fn) const
{
    auto mem_first      = linfo_->data();
    auto const mem_last = mem_first + linfo_->size();

    auto found = fn(mem_first, mem_last);
    return found == mem_last ? nullptr : found;
}

template <class... Segment>
void* library_info::basic_pattern_getter::find(pattern<Segment...> const& pat) const
{
    decltype(auto) front_segment = pat.front().view();
    auto const front_byte        = front_segment.front();

    if constexpr (sizeof...(Segment) == 1)
    {
        if (front_segment.size() == 1)
        {
            auto simple_fn = [front_byte]<typename T>(T const first, T const last) -> T {
                return std::find(first, last, front_byte);
            };
            // return find_anywhere(fn);
            return find_in_section(simple_fn);
        }
    }

    auto full_fn = [front_byte, &pat, pattern_length = pat.length()]<typename T>(T first, T const last) -> T {
        auto const last_safe = last - pattern_length;

        if (first < last_safe)
            for (;;)
            {
                auto const front_byte_found = std::find(first, last_safe, front_byte);
                if (pat.equal(front_byte_found))
                    return front_byte_found;
                if (front_byte_found == last_safe)
                    return last;

                first = front_byte_found + 1;
            }

        if (first == last_safe)
            if (pat.equal(first))
                return first;

        return last;
    };
    // return find_anywhere(full_fn);
    return find_in_section(full_fn);
}
}
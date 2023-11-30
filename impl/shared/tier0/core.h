#pragma once

#define FD_NAMESPACE fd

#define FD_TIER(_NUM_)               FD_NAMESPACE::inline tier##_NUM_
#define FD_TIER_EX(_NUM_, _EX_, ...) FD_TIER(_NUM_)::_EX_ __VA_OPT__(::__VA_ARGS__)

namespace FD_TIER(0)
{
template <unsigned int Length>
consteval bool check_tier(char const (&filename)[Length], char const num)
{
#ifdef _MSC_VER
    auto const path_separator = '\\';
#else
#endif
    char const tier[]      = {path_separator, 't', 'i', 'e', 'r', num, path_separator};
    auto const tier_length = sizeof(tier);

    for (auto filename_offset = 0u; filename_offset != Length - tier_length;)
    {
        if (filename[filename_offset] != tier[0])
        {
            ++filename_offset;
            continue;
        }

        auto tier_offset = 1;
        for (; tier_offset != tier_length; ++tier_offset)
        {
            if (filename[filename_offset + tier_offset] != tier[tier_offset])
            {
                tier_offset = tier_length;
                break;
            }
        }

        if (tier_offset != tier_length)
        {
            return true;
        }
    }

    return false;
}
} // namespace FD_TIER(0)

#ifdef _DEBUG
#define FD_CHECK_TIER(_TIER_) static_assert(FD_NAMESPACE::check_tier(__FILE__, (#_TIER_)[0]));
#else
#define FD_CHECK_TIER(_TIER_)
#endif
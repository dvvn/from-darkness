#include "algorithm/char.h"

namespace fd
{
namespace detail
{
template <class T, size_t Length>
constexpr bool check_chars_range(T const& fn, char const (&arr)[Length], size_t const count = Length - 1)
{
    auto arr_first      = arr;
    auto const arr_last = arr_first + count;

    for (; arr_first != arr_last; ++arr_first)
    {
        if (fn(*arr_first))
            continue;
        return false;
    }
    return true;
}

template <size_t S, size_t S1>
inline constexpr auto default_check_chars_range_count = nullptr;

template <size_t S>
inline constexpr auto default_check_chars_range_count<S, S> = S - 1;

template <class T, typename C_l, typename C_r, size_t Length_l, size_t Length_r>
constexpr bool check_chars_range(
    T const& fn, C_l const (&arr_in)[Length_l], C_r const (&arr_expected)[Length_r], //
    size_t const count = default_check_chars_range_count<Length_l, Length_r>)
{
    for (size_t i = 0; i != count; ++i)
    {
        if (fn(arr_in[i]) == arr_expected[i])
            continue;
        return false;
    }
    return true;
}

inline constexpr auto& lower_chars = "abcdefghijklmnopqrstuvwxyz";
inline constexpr auto& upper_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
inline constexpr auto& digit_chars = "0123456789";
inline constexpr uint8_t digits[]  = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
} // namespace detail

static_assert(check_chars_range(islower, detail::lower_chars));
static_assert(check_chars_range(isupper, detail::upper_chars));
static_assert(check_chars_range(isdigit, detail::digit_chars));
static_assert(check_chars_range(isxdigit, detail::digit_chars));
static_assert(check_chars_range(isxdigit, detail::lower_chars, 6));
static_assert(check_chars_range(isxdigit, detail::upper_chars, 6));
static_assert(check_chars_range(toupper, detail::lower_chars, detail::upper_chars));
static_assert(check_chars_range(tolower, detail::upper_chars, detail::lower_chars));
static_assert(check_chars_range(detail::char_to_num, detail::digit_chars, detail::digits, 10));
static_assert(detail::chars_literal_to_num<uint64_t, '1', '3', '3', '7'>::value == 1337);
} // namespace fd
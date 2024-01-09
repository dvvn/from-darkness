#include "algorithm/char.h"
#include "test_holder.h"

namespace fd
{
namespace detail
{
template <class T, size_t Length>
static bool check_chars_range(T const& fn, char const (&arr)[Length], size_t const count = Length - 1)
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
static bool check_chars_range(
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
} // namespace detail

FD_ADD_TEST([] {
    auto const& lower_chars = "abcdefghijklmnopqrstuvwxyz";
    auto const& upper_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    auto const& digit_chars = "0123456789";
    uint8_t const digits[]  = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

    using detail::char_to_num;

    assert(check_chars_range(islower, lower_chars));
    assert(check_chars_range(isupper, upper_chars));
    assert(check_chars_range(isdigit, digit_chars));
    assert(check_chars_range(isxdigit, digit_chars));
    assert(check_chars_range(isxdigit, lower_chars, 6));
    assert(check_chars_range(isxdigit, upper_chars, 6));
    assert(check_chars_range(toupper, lower_chars, upper_chars));
    assert(check_chars_range(tolower, upper_chars, lower_chars));
    assert(check_chars_range(char_to_num, digit_chars, digits, 10));
    // assert((detail::chars_literal_to_num<uint64_t, '1', '3', '3', '7'>::value) == 1337);
});
} // namespace fd
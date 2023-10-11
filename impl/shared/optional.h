#pragma once

#include <optional>

namespace fd
{
template <typename T>
using optional = std::optional<T>;

#if 0
using std::make_optional;

template <
    typename T, bool =
                    requires(T val) {
                        !val;
                        T{};
                    }>
struct as_optional;

template <typename T>
using as_optional_t = typename as_optional<T>::type;

template <typename T>
struct as_optional<T, false>
{
    using type = optional<T>;
};

template <typename T>
struct as_optional<T, true>
{
    using type = T;
};

template <typename T>
constexpr bool is_optional = false;

template <typename T>
constexpr bool is_optional<optional<T>> = true;

// template <typename T>
// struct as_optional<T, is_optional<T>>
//{
//     using type = T;
// };

template <class T>
constexpr auto to_raw_pointer(T& obj) requires(is_optional<T>)
{
    return obj.operator->();
}
#endif
} // namespace fd
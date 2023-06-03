#pragma once

#include <concepts>
#include <utility>

namespace fd
{
template <typename T>
constexpr size_t get_hash(T const &val, std::hash<T> hasher = {})
{
    return hasher(val);
}

template <typename T>
struct hashed_object : std::pair<T, size_t>
{
    template <typename Q>
    friend struct hashed_object;

    hashed_object() = default;

    using std::pair<T, size_t>::first;
    using std::pair<T, size_t>::second;

    // using std::pair<T, size_t>::pair;

    template <typename Q = T>
    constexpr hashed_object(Q &&obj) requires(std::constructible_from<T, Q>)
        : std::pair<T, size_t>(std::forward<Q>(obj), 0)
    {
        second = get_hash(first);
    }

    template <typename Q>
    /*explicit*/ constexpr hashed_object(hashed_object<Q> const &other)
        requires(std::constructible_from<T, Q> && !std::same_as<T, Q>)
        : std::pair<T, size_t>(other.first, other.second)
    {
    }

    template <typename Q = T>
    constexpr hashed_object(Q &&obj, size_t hash)
        : std::pair<T, size_t>(std::forward<Q>(obj), hash)
    {
    }

    T *operator->()
    {
        return &first;
    }

    template <typename Q = T>
    bool operator==(hashed_object<Q> const &other) const requires requires() { first == other.first; }
    {
        return second == other.second;
    }

    template <typename Q = T>
    bool operator==(Q const &other) const requires requires() { first == other; }
    {
        return first == other;
    }
};

template <typename T>
constexpr size_t get_hash(hashed_object<T> const &obj)
{
    return get<size_t>(obj);
}

template <typename T>
hashed_object(T) -> hashed_object<std::decay_t<T>>;

template <typename Q, std::convertible_to<Q> T>
constexpr Q get(hashed_object<T> const &obj) noexcept requires(!std::same_as<Q, T>)
{
    return std::get<T>(obj);
}

} // namespace fd
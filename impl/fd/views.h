#pragma once

#include <iterator>

namespace fd
{
template <class It>
static decltype(auto) _unwrap(It it)
{
    if constexpr (std::_Unwrappable_v<It>)
        return std::_Get_unwrapped(it);
    else
        return it;
}

template <class T>
concept have_unchecked = requires(T& obj) {
                             obj._Unchecked_begin();
                             obj._Unchecked_end();
                         };

template <class T>
concept have_data = requires(T& obj) {
                        obj.data();
                        obj.size();
                    };

template <typename T>
static constexpr auto _begin(T& container)
{
    if constexpr (have_unchecked<T>)
        return container._Unchecked_begin();
    else if constexpr (have_data<T>)
        return container.data();
    else
        return _unwrap(std::begin(container));
}

template <typename T>
static constexpr auto _end(T& container)
{
    if constexpr (have_unchecked<T>)
        return container._Unchecked_end();
    else if constexpr (have_data<T>)
        return container.data() + container.size();
    else
        return _unwrap(std::end(container));
}

#if 1
// ReSharper disable once CppInconsistentNaming
constexpr auto begin = [](auto& container) {
    return _begin(container);
};
// ReSharper disable once CppInconsistentNaming
constexpr auto end = [](auto& container) {
    return _end(container);
};
// ReSharper disable once CppInconsistentNaming
constexpr auto rbegin = [](auto& container) {
    return std::reverse_iterator(_begin(container));
};
// ReSharper disable once CppInconsistentNaming
constexpr auto rend = [](auto& container) {
    return std::reverse_iterator(_end(container));
};

#else
constexpr auto begin(auto& container)
{
    return _begin(container);
}

constexpr auto end(auto& container)
{
    return _end(container);
}

constexpr auto rbegin(auto& container)
{
    return std::reverse_iterator(_begin(container));
}

constexpr auto rend(auto& container)
{
    return std::reverse_iterator(_end(container));
}
#endif

template <typename T>
concept native_iterable = requires(T container) {
                              begin(container);
                              end(container);
                          };

template <typename T>
class range_view;

template <typename T>
class range_view
{
    T begin_, end_;

  public:
    constexpr range_view(T begin, T end)
        : begin_(std::move(begin))
        , end_(std::move(end))
    {
    }

    constexpr range_view(T begin, size_t size)
        // ReSharper disable once CppMemberInitializersOrder
        : end_(begin + size)
        , begin_(std::move(begin))

    {
    }

    constexpr T begin() const
    {
        return begin_;
    }

    constexpr T end() const
    {
        return end_;
    }

    constexpr size_t size() const
    {
        return std::distance(begin_, end_);
    }
};

template <typename Container>
constexpr auto forward_view(Container& container) -> range_view<decltype(begin(container))>
{
    return { begin(container), end(container) };
}

template <typename Container>
constexpr auto reverse_view(Container& container) -> range_view<decltype(rbegin(container))>
{
    return { rbegin(container), rend(container) };
}
} // namespace fd
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
concept have_begin_end = requires(T container) {
                             std::begin(container);
                             std::end(container);
                         };

template <typename T>
static constexpr auto _begin(T& container)
{
    if constexpr (have_unchecked<T>)
        return container._Unchecked_begin();
    else if constexpr (have_data<T>)
        return container.data();
    else if constexpr (have_begin_end<T>)
        return _unwrap(std::begin(container));
}

template <typename T>
static constexpr auto _end(T& container)
{
    if constexpr (have_unchecked<T>)
        return container._Unchecked_end();
    else if constexpr (have_data<T>)
        return container.data() + container.size();
    else if constexpr (have_begin_end<T>)
        return _unwrap(std::end(container));
}

template <typename T>
concept native_iterable = have_unchecked<T> || have_data<T> || have_begin_end<T>;

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

#if 1
template <class T, uint8_t Mode>
class range_view_lazy;

template <class T, uint8_t Mode>
class range_view_lazy<T&&, Mode>
{
  public:
    range_view_lazy() = delete;
};

template <class T>
class forward_view_lazy
{
    T* source_;

  public:
    constexpr forward_view_lazy(T& source)
        : source_(&source)
    {
    }

    constexpr decltype(auto) begin() const
    {
        return (_begin(*source_));
    }

    constexpr decltype(auto) end() const
    {
        return (_end(*source_));
    }
};

template <class T>
class reverse_view_lazy
{
    T* source_;

  public:
    constexpr reverse_view_lazy(T& source)
        : source_(&source)
    {
    }

    constexpr auto begin() const
    {
        return std::reverse_iterator(_end(*source_));
    }

    constexpr auto end() const
    {
        return std::reverse_iterator(_begin(*source_));
    }
};

#endif

template <typename Container>
constexpr auto forward_view(Container& container)
{
    return range_view(_begin(container), _end(container));
}

template <typename Container>
constexpr auto reverse_view(Container& container)
{
    return range_view(std::reverse_iterator(_end(container)), std::reverse_iterator(_begin(container)));
}
} // namespace fd
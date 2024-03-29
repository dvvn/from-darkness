#pragma once

#include <iterator>

namespace fd
{
template <class It>
constexpr decltype(auto) decay_iter(It it)
{
    if constexpr (std::_Unwrappable_v<It>)
        return std::_Get_unwrapped(it);
    else
        return it;
}

template <typename T>
concept iterator = requires(T it) {
                       *it;
                       ++it;
                   };

template <class T>
concept have_unchecked = requires(T& obj) {
                             obj._Unchecked_begin();
                             obj._Unchecked_end();
                         };

template <class T>
concept have_size = requires(T& obj) { std::size(obj); };

template <class T>
concept have_data = requires(T& obj) {
                        std::data(obj);
                        std::size(obj);
                    };

template <typename T>
concept have_begin_end = requires(T container) {
                             std::begin(container);
                             std::end(container);
                         };

template <class T>
concept have_empty = requires(T& obj) { std::empty(obj); };

template <typename T>
static constexpr auto _begin(T& container)
{
    if constexpr (have_unchecked<T>)
        return container._Unchecked_begin();
    else if constexpr (have_data<T>)
        return std::data(container);
    else if constexpr (have_begin_end<T>)
        return decay_iter(std::begin(container));
}

template <typename T>
static constexpr auto _end(T& container)
{
    if constexpr (have_unchecked<T>)
        return container._Unchecked_end();
    else if constexpr (have_data<T>)
        return std::data(container) + std::size(container);
    else if constexpr (have_begin_end<T>)
        return decay_iter(std::end(container));
}

template <typename T>
static constexpr size_t _size(T&& container)
{
    if constexpr (have_size<T>)
        return std::size(container);
    else if constexpr (have_unchecked<T>)
        return std::distance(container._Unchecked_begin(), container._Unchecked_end());
    else if constexpr (have_begin_end<T>)
        return std::distance(_begin(container), _end(container));
}

template <typename T>
static constexpr auto _size_or_end(T& container)
{
    if constexpr (have_size<T>)
        return static_cast<size_t>(std::size(container));
    else
        return _end(container);
}

template <typename T>
static constexpr auto _empty(T& container)
{
    if constexpr (have_empty<T>)
        return std::empty(container);
    else if constexpr (have_size<T>)
        return std::size(container) == 0;
    else
        return _begin(container) == _end(container);
}

template <typename T>
concept native_iterable = have_unchecked<T> || have_data<T> || have_begin_end<T>;

template <typename T>
struct _iterator_type
{
    using range_type = std::remove_reference_t<T>&;
    using begin_type = std::remove_cvref_t<decltype(_begin(std::declval<range_type>()))>;

#ifdef _DEBUG
    using end_type = std::remove_cvref_t<decltype(_end(std::declval<range_type>()))>;
    using type     = std::conditional_t<std::same_as<begin_type, end_type>, begin_type, void>;
#else
    using type = begin_type;
#endif
};

template <typename T>
using iter_t = typename _iterator_type<T>::type;

template <typename T>
class range_view;

template <typename T>
static void _range_view_filter(const range_view<T>&)
{
}

template <typename T>
concept range_view_accepter = requires(T rng) { _range_view_filter(rng); };

template <typename T>
class range_view
{
    T begin_, end_;

    template <typename T2>
    friend class range_view;

  public:
    constexpr range_view() = default;

    template <native_iterable Rng>
    constexpr range_view(Rng&& rng) requires(!range_view_accepter<Rng> && std::constructible_from<T, iter_t<Rng>>) // NOLINT(bugprone-forwarding-reference-overload)
        : begin_(_begin(rng))
        , end_(_end(rng))
    {
#ifdef _DEBUG
        static_assert(std::is_trivially_destructible_v<Rng> || !std::is_rvalue_reference_v<Rng&&>);
#endif
    }

    template <range_view_accepter Rng>
    constexpr range_view(Rng&& rng) requires(std::constructible_from<T, iter_t<Rng>>) // NOLINT(bugprone-forwarding-reference-overload)
        : begin_(std::forward_like<Rng&&>(rng.begin_))
        , end_(std::forward_like<Rng&&>(rng.end_))
    {
    }

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

    constexpr range_view(T begin, range_view end)
        : begin_(std::move(begin), end_(std::move(end.end_)))
    {
    }

    constexpr range_view(size_t offset, range_view rng)
        : begin_(rng.begin_ + offset)
        , end_(std::move(rng.end_))
    {
    }

    constexpr range_view(range_view begin, T end)
        : begin_(std::move(begin.begin_))
        , end_(std::move(end))
    {
    }

    constexpr range_view(range_view begin, size_t size)
        // ReSharper disable once CppMemberInitializersOrder
        : end_(begin.begin_ + size)
        , begin_(std::move(begin.begin_))
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

#if 0
    constexpr size_t size() const
    {
        return std::distance(begin_, end_);
    }

    constexpr bool empty() const
    {
        return begin_ == end_;
    }

    constexpr range_view subrange(const size_t offset) const
    {
        return { begin_ + offset, end_ };
    }

    constexpr range_view subrange(const T newBegin) const
    {
        return { newBegin, end_ };
    }

    constexpr auto distance(const T end) const
    {
        return std::distance(begin_, end);
    }
#endif
};

template <typename Rng>
range_view(Rng&) -> range_view<iter_t<Rng>>;

template <native_iterable Rng>
range_view(Rng&&, iter_t<Rng>) -> range_view<iter_t<Rng>>;

template <native_iterable Rng>
range_view(Rng&&, size_t) -> range_view<iter_t<Rng>>;

template <typename T>
class range_view_creator
{
    T& rng_;

  public:
    using range = range_view<iter_t<T>>;

    constexpr range_view_creator(T& rng)
        : rng_(rng)
    {
    }

    constexpr range drop(size_t count) const
    {
        return { _begin(rng_), _size_or_end(rng_) - count };
    }

    constexpr range skip(size_t count) const
    {
        return { _begin(rng_) + count, _end(rng_) };
    }

    template <typename It>
    constexpr range from(It it) const
    {
        return { decay_iter(it), _end(rng_) };
    }

    template <typename It>
    constexpr range to(It it) const
    {
        return { _begin(rng_), decay_iter(it) };
    }
};

template <typename T, typename T2>
using select_const_t = std::conditional_t<std::is_const_v<T>, T, T2>;

template <native_iterable Rng, iterator It>
range_view(Rng&, It) -> range_view<select_const_t<It, iter_t<Rng>>>;

template <iterator It, native_iterable Rng>
range_view(It, Rng&) -> range_view<select_const_t<It, iter_t<Rng>>>;

template <typename It>
class reverse_iterator
{
    // std::reverse_iterator without noexcept, explicit, tags and other shit

    It it_;

  public:
    template <typename>
    friend class reverse_iterator;

    constexpr reverse_iterator(It it)
        : it_(it)
    {
    }

    template <typename Other>
    constexpr reverse_iterator(reverse_iterator<Other> other)
        : it_(other.it_)
    {
    }

    //---------

    constexpr reverse_iterator& operator++()
    {
        --it_;
        return *this;
    }

    constexpr reverse_iterator operator++(int)
    {
        reverse_iterator tmp = *this;
        --it_;
        return tmp;
    }

    constexpr reverse_iterator& operator--()
    {
        ++it_;
        return *this;
    }

    constexpr reverse_iterator operator--(int)
    {
        auto tmp = *this;
        ++it_;
        return tmp;
    }

    constexpr reverse_iterator& operator+=(const ptrdiff_t diff)
    {
        it_ -= diff;
        return *this;
    }

    constexpr reverse_iterator operator-(const ptrdiff_t diff) const
    {
        return it_ + diff;
    }

    constexpr reverse_iterator& operator-=(const ptrdiff_t diff)
    {
        it_ += diff;
        return *this;
    }

    constexpr decltype(auto) operator[](const ptrdiff_t diff) const
    {
        return it_[-diff - 1];
    }

    constexpr decltype(auto) operator*() const
    {
        auto tmp = it_;
        return *--tmp;
    }

    constexpr auto operator->() const
    {
        if constexpr (std::is_class_v<It>)
            return it_.operator->();
        else
            return it_;
    }

    //---------

    constexpr It base() const
    {
        return it_;
    }

    template <typename Other>
    constexpr bool operator==(reverse_iterator<Other> other) const
    {
        return it_ == other.it_;
    }
};

template <typename It>
reverse_iterator(It) -> reverse_iterator<It>;

template <native_iterable Rng>
[[nodiscard]] constexpr auto reversed(Rng&& range) -> range_view<reverse_iterator<iter_t<Rng>>>
{
#ifdef _DEBUG
    static_assert(std::is_trivially_destructible_v<Rng> || !std::is_rvalue_reference_v<Rng&&>);
#endif
    return { _end(range), _begin(range) };
}

template <typename T>
[[nodiscard]] constexpr auto reversed(range_view<reverse_iterator<T>> rng) -> range_view<T>
{
    return { rng.begin().base(), rng.end().base() };
}

} // namespace fd
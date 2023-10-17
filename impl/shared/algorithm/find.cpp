#include "algorithm/find.h"
#include "search_stop_token.h"
#include "algorithm/search.h"
#include "container/array.h"
#include "container/vector/dynamic.h"
#include "diagnostics/fatal.h"
#include "functional/cast.h"
#include "functional/ignore.h"
#include "memory/basic_xref.h"
#include "memory/pattern_allocator.h"

#include <algorithm>
#include <cassert>

#if 0
template <>
struct std::iterator_traits<fd::basic_pattern::iterator>
{
    // ReSharper disable once CppInconsistentNaming
    using _It = fd::basic_pattern::iterator;

    using pointer           = _It::pointer;
    using reference         = _It::reference;
    using iterator_category = forward_iterator_tag;
    using value_type        = decay_t<reference>;
    using difference_type   = ptrdiff_t;
};
#endif

namespace fd
{
struct pattern_segment_view
{
    using pointer         = basic_pattern_segment::pointer;
    using size_type       = basic_pattern_segment::size_type;
    using difference_type = basic_pattern_segment::difference_type;

    using iterator = pointer;

  private:
    pointer begin_;
    pointer end_;
#ifdef _DEBUG
    size_type length_;
#endif
    size_type tail_;

  public:
    pattern_segment_view()
    {
        ignore_unused(this);
    }

    pattern_segment_view(basic_pattern_segment const* segment)
        : begin_(segment->begin())
        , end_(segment->end())
#ifdef _DEBUG
        , length_(std::distance(begin_, end_))
#endif
        , tail_(segment->tail())

    {
    }

    iterator begin() const
    {
        return begin_;
    }

    iterator end() const
    {
        return end_;
    }

    pointer data() const
    {
        return begin_;
    }

    size_type tail() const
    {
        return tail_;
    }

    size_type length() const
    {
#ifdef _DEBUG
        return length_;
#else
        return static_cast<size_type>(std::distance(begin_, end_));
#endif
    }

    size_type abs_length() const
    {
        return length() + tail();
    }
};

template <bool Rewrap>
static uint8_t* find_byte(void* begin, void const* end, uint8_t const byte)
{
    auto const it = std::find(safe_cast<uint8_t*>(begin), unsafe_cast<uint8_t*>(end), byte);
    if constexpr (Rewrap)
    {
        if (it == end)
            return nullptr;
    }
    return it;
}

template <bool Rewrap, typename T>
static uint8_t* search_byte(void* begin, void const* end, uint8_t const byte, T const& validate)
{
    for (;;)
    {
        auto const it = find_byte<false>(begin, end, byte);
        if (it == end)
            return Rewrap ? nullptr : it;

        if (!validate(it))
        {
            begin = it;
            continue;
        }
        return it;
    }
}

static auto distance(void const* begin, void const* end)
{
    assert(begin < end);
    return std::distance(safe_cast<uint8_t>(begin), safe_cast<uint8_t>(end));
}

template <bool ValidateTail, typename Fn = nullptr_t>
static uint8_t* search_segment(void* begin, void const* end, pattern_segment_view const& segment, Fn const& filter = {})
{
    for (auto b_begin = safe_cast<uint8_t>(begin);;)
    {
        auto const midpoint = find_byte<false>(b_begin, end, segment.begin()[0]);
        if (midpoint == end)
            return nullptr;

        if (memcmp(midpoint, segment.begin(), segment.length()) != 0)
        {
            b_begin = midpoint + 1;
            continue;
        }

        if constexpr (std::invocable<Fn, uint8_t*>)
        {
            if (!filter(midpoint))
            {
                b_begin = midpoint + segment.abs_length();
                continue;
            }
        }
        if constexpr (ValidateTail)
        {
            if (segment.tail() != 0)
                if (distance(midpoint, end) > segment.abs_length())
                    return nullptr;
        }
        return midpoint;
    }
}

static uint8_t first_byte(void const* ptr)
{
    return safe_cast<uint8_t>(ptr)[0];
}

template <typename Fn = nullptr_t>
static void* scan_range(
    void* begin, void const* end,                                 //
    void const* from, void const* to, size_t const target_length, //
    Fn const& filter = {})
{
    assert(target_length > 1);
    assert(target_length == distance(from, to)); // NOLINT(clang-diagnostic-sign-compare)

    auto const from_0   = first_byte(from);
    auto const safe_end = safe_cast<uint8_t>(end) - target_length;

    for (auto b_begin = safe_cast<uint8_t>(begin);;)
    {
        auto const front = find_byte<false>(b_begin, safe_end, from_0);
        if (front == safe_end)
            return nullptr;

        if (memcmp(front, from, target_length) == 0)
        {
            if constexpr (std::invocable<Fn, uint8_t*>)
                if (!filter(front))
                {
                    b_begin = front + target_length;
                    continue;
                }

            return front;
        }

        b_begin = front + 1;
    }
}

template <class T>
struct basic_pattern_view
{
    template <size_t Size>
    friend struct pattern_view;

    using pointer       = pattern_segment_view*;
    using const_pointer = pattern_segment_view const*;
    using reference     = pattern_segment_view const&;

    using iterator         = const_pointer;
    using reverse_iterator = std::reverse_iterator<const_pointer>;

    using size_type       = basic_pattern::size_type;
    using difference_type = basic_pattern::difference_type;

  private:
    T buff_;
    size_type abs_length_;

  protected:
    pointer data()
    {
        return std::data(buff_);
    }

    void set_abs_length()
    {
        switch (std::size(buff_))
        {
        case 0:
            unreachable();
            // ReSharper disable once CppUnreachableCode
            break;
        case 1:
            abs_length_ = begin()->abs_length();
            break;
        default:
            abs_length_   = 0;
            auto const ed = end();
            for (auto it = begin(); it != ed; ++it)
                abs_length_ += it->abs_length();
        }
    }

  public:
    basic_pattern_view()
    {
        static_assert(std::ranges::random_access_range<T>);
        ignore_unused(this);
    }

    iterator begin() const
    {
#ifdef _DEBUG
        return std::data(buff_);
#else
        return iterator_to_raw_pointer(std::begin(buff_));
#endif
    }

    iterator end() const
    {
#ifdef _DEBUG
        return std::data(buff_) + std::size(buff_);
#else
        return iterator_to_raw_pointer(std::end(buff_));
#endif
    }

    reverse_iterator rbegin() const
    {
        return reverse_iterator(end());
    }

    reverse_iterator rend() const
    {
        return reverse_iterator(begin());
    }

    reference operator[](ptrdiff_t pos) const
    {
#ifdef _DEBUG
        return std::data(buff_)[pos];
#else
        return buff_[pos];
#endif
    }

    bool have_tail() const
    {
#ifdef _DEBUG
        return rbegin()->tail();
#else
        return buff_.back().tail();
#endif
    }

    size_type abs_length() const
    {
        return abs_length_;
    }
};

template <size_t SegmentCount>
struct pattern_view final : basic_pattern_view<array<pattern_segment_view, SegmentCount>>
{
    pattern_view(basic_pattern const& pattern)
    {
        assert(pattern.segments() == SegmentCount);
        auto dst       = this->data();
        auto const end = pattern.end();
        for (auto it = pattern.begin(); it != end; ++it)
            std::construct_at(dst++, *it);
        this->set_abs_length();
    }
};

template <>
struct pattern_view<0> final : basic_pattern_view<vector_ex2<pattern_segment_view, detail::dynamic_pattern_allocator>>
{
    pattern_view(basic_pattern const& pattern)
    {
        this->buff_.assign(pattern.begin(), pattern.end());
        this->set_abs_length();
    }
};

template <>
struct pattern_view<1>; // deleted

template <size_t SegmentCount, typename Fn = nullptr_t>
static void* search_pattern(void* begin, void const* end, pattern_view<SegmentCount> const& view, Fn const& filter = {})
{
    auto const first_segment = view.begin();
    auto const last_segment  = view.end();

    auto const first_segment_length     = first_segment->length();
    auto const first_segment_abs_length = first_segment->abs_length();

    for (auto b_begin = safe_cast<uint8_t>(begin);;)
    {
        auto const first_match = search_segment<false>(b_begin, end, first_segment[0]);
        if (first_match)
        {
            bool compared;

            auto match        = first_match + first_segment_abs_length;
            auto next_segment = first_segment + 1;
            for (;;)
            {
                if (memcmp(match, next_segment->data(), next_segment->length()) != 0)
                {
                    compared = false;
                    break;
                }
                if (next_segment + 1 == last_segment)
                {
                    compared = true;
                    break;
                }
                match += next_segment->abs_length();
                ++next_segment;
            }
            if (!compared)
            {
                b_begin = first_match + first_segment_length;
                continue;
            }

            if constexpr (std::invocable<Fn, uint8_t*>)
            {
                if (!filter(first_match))
                {
                    b_begin = first_match + view.abs_length();
                    continue;
                }
            }

            if (view.have_tail())
            {
                if (distance(first_match, end) > view.abs_length())
                    return nullptr;
            }
        }
        return first_match;
    }
}

template <size_t SegmentCount, typename Fn = nullptr_t>
static void* select_search(void* begin, void const* end, basic_pattern const& pattern, Fn const& filter = {})
{
    if constexpr (SegmentCount == 0)
        unreachable();
    else if constexpr (SegmentCount == 1)
        return search_segment<true>(begin, end, *pattern.begin(), filter);
    else
        return search_pattern<SegmentCount>(begin, end, pattern, filter);
}

constexpr size_t cached_segments_limit = 32;

template <typename FnGetter>
constexpr auto cache_segments(FnGetter getter)
{
    using fn_t                = decltype(getter(std::in_place_index<0>));
    constexpr auto array_size = cached_segments_limit + 1;

    return [&]<size_t... I>(std::index_sequence<I...>) -> array<fn_t, array_size> {
        return {getter(std::in_place_index<I>)...};
    }(std::make_index_sequence<array_size>());
}

void* find(void* begin, void const* end, basic_pattern const& pattern)
{
    constexpr auto cache = cache_segments([]<size_t I>(std::in_place_index_t<I>) {
        return select_search<I>;
    });
    void* found;
    if (auto const segments = pattern.segments(); segments < cache.size())
    {
        found = cache[segments] // NOLINT(cppcoreguidelines-pro-bounds-constant-array-index)
            (begin, end, pattern, nullptr);
    }
    else
    {
        found = search_pattern<0>(begin, end, pattern);
    }
    return found;
}

void* find(void* begin, void const* end, basic_xref const& xref)
{
    auto const ptr = xref.get();
    return scan_range(begin, end, ptr, ptr + 1, sizeof(basic_xref::value_type));
}

void* find(void* begin, void const* end, void const* from, void const* to)
{
    void* found;
    if (auto const target_length = distance(from, to); target_length == 1)
        found = find_byte<true>(begin, end, first_byte(from));
    else
        found = scan_range(begin, end, from, to, target_length);
    return found;
}

void* search(void* begin, void const* end, basic_pattern const& pattern, search_stop_token const& token)
{
    constexpr auto cache = cache_segments([]<size_t I>(std::in_place_index_t<I>) {
        return select_search<I, search_stop_token>;
    });
    void* found;
    if (auto const segments = pattern.segments(); segments < cache.size())
    {
        found = cache[segments] // NOLINT(cppcoreguidelines-pro-bounds-constant-array-index)
            (begin, end, pattern, token);
    }
    else
    {
        found = search_pattern<0>(begin, end, pattern, token);
    }
    return found;
}

void* search(void* begin, void const* end, basic_xref const& xref, search_stop_token const& token)
{
    auto const ptr = xref.get();
    return scan_range(begin, end, ptr, ptr + 1, sizeof(basic_xref::value_type), token);
}

void* search(void* begin, void const* end, void const* from, void const* to, search_stop_token const& token)
{
    void* found;
    if (auto const target_length = distance(from, to); target_length == 1)
        found = search_byte<true>(begin, end, first_byte(from), token);
    else
        found = scan_range(begin, end, from, to, target_length, token);
    return found;
}
} // namespace fd
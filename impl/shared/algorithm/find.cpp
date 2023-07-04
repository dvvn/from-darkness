#include "basic_pattern.h"
#include "basic_xref.h"
#include "find.h"
#include "type_traits.h"
#include "container/array.h"
#include "container/vector/dynamic.h"

#include <algorithm>
#include <cassert>

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

namespace fd
{
struct pattern_segment_view
{
    using pointer   = basic_pattern_segment::pointer;
    using size_type = std::make_unsigned_t<std::iter_difference_t<pointer>>;
    using tail_type = basic_pattern_segment::size_type;

  private:
    pointer begin_;
    pointer end_;
    tail_type tail_;
#ifdef _DEBUG
    size_type length_;
    size_type abs_length_;
#endif

  public:
    pattern_segment_view()
    {
        ignore_unused(this);
    }

    pattern_segment_view(basic_pattern_segment const &segment)
        : begin_(segment.begin())
        , end_(segment.end())
        , tail_(segment.tail())
#ifdef _DEBUG
        , length_(std::distance(begin_, end_))
        , abs_length_(length_ + tail_)
#endif
    {
    }

    pointer begin() const
    {
        return begin_;
    }

    pointer end() const
    {
        return end_;
    }

    tail_type tail() const
    {
        return tail_;
    }

    size_type length() const
    {
#ifdef _DEBUG
        return length_;
#else
        return std::distance(begin_, end_);
#endif
    }

    size_type abs_length() const
    {
#ifdef _DEBUG
        return abs_length_;
#else
        return length() + tail_;
#endif
    }
};

template <int, typename... T>
requires(sizeof(T) == -1)
static void *find(T...)
{
    // dummy gap
    std::unreachable();
}

template <bool Rewrap>
static uint8_t *find_byte(void *begin, void *end, uint8_t byte)
{
    auto it = std::find(static_cast<uint8_t *>(begin), static_cast<uint8_t *>(end), byte);
    if constexpr (Rewrap)
    {
        if (it == end)
            return nullptr;
    }
    return it;
}

static ptrdiff_t distance(void const *begin, void const *end)
{
    return std::distance(static_cast<uint8_t const *>(begin), static_cast<uint8_t const *>(end));
}

template <bool ValidateTail>
static uint8_t *find(void *begin, void *end, pattern_segment_view const &segment)
{
    auto b_begin = static_cast<uint8_t *>(begin);

    for (;;)
    {
        auto midpoint = find_byte<false>(b_begin, end, *segment.begin());
        if (midpoint == end)
            return nullptr;

        if (memcmp(midpoint, segment.begin(), segment.length()) != 0)
        {
            b_begin = midpoint + 1;
            continue;
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

void *find(void *begin, void *end, basic_pattern_segment const &segment)
{
    return find<true>(begin, end, segment);
}

template <class T>
class basic_pattern_view
{
    template <size_t Size>
    friend struct pattern_view;

    T buff_;

  public:
    basic_pattern_view()
    {
        static_assert(std::ranges::random_access_range<T>);
    }

    using pointer   = pattern_segment_view const *;
    using reference = pattern_segment_view const &;
    using iterator  = pointer;

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

    std::reverse_iterator<iterator> rbegin() const
    {
        return end();
    }

    std::reverse_iterator<iterator> rend() const
    {
        return begin();
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
        return buff_.back().tail();
    }

    size_t abs_length() const
    {
        size_t ret = 0;
        auto ed    = end();
        for (auto it = begin(); it != ed; ++it)
            ret += it->abs_length();
        return ret;
    }
};

template <size_t Size>
struct pattern_view : basic_pattern_view<array<pattern_segment_view, Size>>
{
    pattern_view(basic_pattern const &pattern)
    {
        assert(pattern.size() == Size);
        auto dst = this->buff_.data();
        auto end = pattern.end();
        for (auto it = pattern.begin(); it != end; ++it)
            std::construct_at(dst++, *it);
    }
};

template <>
struct pattern_view<0> : basic_pattern_view<vector<pattern_segment_view>>
{
    pattern_view(basic_pattern const &pattern)
    {
        this->buff_.assign(pattern.begin(), pattern.end());
    }
};

template <>
struct pattern_view<1>; // deleted

template <size_t SegmentCount>
static void *find_pattern(void *begin, void *end, basic_pattern const &pattern)
{
    auto b_begin = static_cast<uint8_t *>(begin);

    pattern_view<SegmentCount> view(pattern);

    auto segments_begin = view.begin();
    auto segments_end   = view.end();

    for (;;)
    {
        auto first_match = find<false>(b_begin, end, *segments_begin);
        if (first_match)
        {
            auto match        = first_match + segments_begin->abs_length();
            auto next_segment = segments_begin + 1;
            bool pass;
            for (;;)
            {
                if (memcmp(match, next_segment->begin(), next_segment->length()) != 0)
                {
                    pass = false;
                    break;
                }
                if (next_segment + 1 == segments_end)
                {
                    pass = true;
                    break;
                }
                match += next_segment->abs_length();
                ++next_segment;
            }
            if (!pass)
            {
                b_begin = first_match + segments_begin->length();
                continue;
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

template <size_t SegmentCount>
static void *find(void *begin, void *end, basic_pattern const &pattern)
{
    if constexpr (SegmentCount == 0)
        std::unreachable();
    else if constexpr (SegmentCount == 1)
        return find<true>(begin, end, *pattern.begin());
    else
        return find_pattern<SegmentCount>(begin, end, pattern);
}

using find_pattern_t = void *(*)(void *, void *, basic_pattern const &);

template <size_t SegmentCount>
class known_pattern_segements
{
    static constexpr size_t table_size_ = SegmentCount + 1; // +1 to skip 0
#ifdef _DEBUG
    std::array<find_pattern_t, table_size_> table_;
#else
    find_pattern_t table_[table_size_];
#endif

  public:
    template <size_t... I>
    consteval known_pattern_segements(std::index_sequence<I...>)
        : table_{find<I>...}
    {
    }

    consteval known_pattern_segements()
        : known_pattern_segements(std::make_index_sequence<table_size_>())
    {
    }

    find_pattern_t operator[](size_t index) const
    {
        return table_[index];
    }

    size_t size() const
    {
        ignore_unused(this);
        return table_size_;
    }
};

void *find(void *begin, void *end, basic_pattern const &pattern)
{
    auto pattern_segments = pattern.size();

    constexpr known_pattern_segements<32> cache;
    auto use_cache = pattern_segments < cache.size();
    return (use_cache ? cache[pattern_segments] : find_pattern<0>)(begin, end, pattern);
}

void *find(void *begin, void *end, basic_xref const &xref)
{
    auto ptr = xref.get();
    return find(begin, end, ptr, ptr + 1);
}

void *find(void *begin, void *end, void const *from, void const *to)
{
    auto from_0        = static_cast<uint8_t const *>(from)[0];
    auto target_length = distance(from, to);

    if (target_length == 1)
    {
        return find_byte<true>(begin, end, from_0);
    }

    auto b_begin = static_cast<uint8_t *>(begin);
    auto b_end   = static_cast<uint8_t *>(end);

    for (auto safe_end = b_end - target_length;;)
    {
        auto front = find_byte<false>(b_begin, safe_end, from_0);
        if (front == safe_end)
            return nullptr;

        if (memcmp(front, from, target_length) == 0)
            return front;

        b_begin = front + 1;
    }
}
}
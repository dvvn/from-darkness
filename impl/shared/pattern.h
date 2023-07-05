#pragma once

#include "basic_pattern.h"
#include "container/array.h"
#include "container/span.h"
#include "container/vector/dynamic.h"
#include "string/charconv.h"
#include "string/view.h"

#include <boost/hana/append.hpp>
#include <boost/hana/sum.hpp>
#include <boost/hana/tuple.hpp>

#include <algorithm>
#include <cassert>
#include <ranges>

namespace fd
{
template <size_t S>
struct pattern_string
{
    char buff[S - 1];

    template <size_t... I>
    consteval pattern_string(char const *buff, std::index_sequence<I...>)
        : buff{buff[I]...}
    {
    }

    consteval pattern_string(char const (&buff)[S])
        : pattern_string(buff, std::make_index_sequence<S - 1>())
    {
    }
};

namespace detail
{
template <typename T>
struct dynamic_segment_allocator : std::allocator<T>
{
    using size_type       = basic_pattern_segment::size_type;
    using difference_type = std::make_signed_t<size_type>;
};
} // namespace detail

class dynamic_pattern_segment final : public basic_pattern_segment
{
    using buffer_type = vector_ex2<value_type, detail::dynamic_segment_allocator>;

    buffer_type buffer_;
    size_type tail_;

  public:
    constexpr dynamic_pattern_segment()
        : tail_(0)
    {
    }

    constexpr char const *fill(char const *pattern, char const *pattern_end)
    {
        for (auto part : span(pattern, pattern_end) | std::views::lazy_split(' '))
        {
            auto it = part.begin();

            if (*it == '?')
            {
                ++tail_;
                continue;
            }

            auto part_begin = &*it;

            // next part detected
            if (tail_ != 0)
                return part_begin;

            auto part_end = part_begin + std::ranges::distance(part);

            value_type value = 0;

            auto result = from_chars(part_begin, part_end, value, 16);
            if (result.ec != std::errc())
                std::unreachable();

            buffer_.emplace_back(value);
        }

        return pattern_end;
    }

    constexpr std::pair<pointer, size_type> extract() const
    {
        return {buffer_.data(), tail_};
    }

    constexpr pointer data() const
    {
        return buffer_.data();
    }

    pointer begin() const override
    {
        return buffer_.data();
    }

    pointer end() const override
    {
        return buffer_.data() + buffer_.size();
    }

    size_type tail() const override
    {
        return tail_;
    }

    constexpr size_type length() const
    {
        return buffer_.size();
    }

    constexpr size_type abs_length() const
    {
        return length() + tail_;
    }

  protected:
    size_type self_size() const override
    {
        return sizeof(dynamic_pattern_segment);
    }
};

class dynamic_pattern final : public basic_pattern
{
    friend constexpr size_t size(dynamic_pattern const &p);

    using segment_type = dynamic_pattern_segment;
    using buffer_type  = vector_ex2<segment_type, detail::dynamic_segment_allocator>;

    buffer_type buffer_;

  public:
    constexpr dynamic_pattern(char const *pat, size_t pattern_length)
    {
        for (auto end = pat + pattern_length; pat != end;)
        {
            auto &segment = buffer_.emplace_back();
            pat           = segment.fill(pat, end);
        }
    }

    size_t size() const override
    {
        return buffer_.size();
    }

    constexpr size_t length() const
    {
        size_t ret = 0;
        for (auto &s : buffer_)
            ret += s.length();
        return ret;
    }

    /*size_t abs_length() const override
    {
        auto rng = buffer_ | std::views::transform(&segment_type::abs_length);
        return std::accumulate(rng.begin(), rng.end(), 0);
    }*/

    iterator begin() const override
    {
        return buffer_.data();
    }

    iterator end() const override
    {
        return buffer_.data() + buffer_.size();
    }

    constexpr dynamic_pattern_segment const &operator[](ptrdiff_t index) const
    {
        return buffer_[index];
    }
};

constexpr size_t size(dynamic_pattern const &p)
{
    return p.buffer_.size();
}

template <size_t S>
constexpr dynamic_pattern make_pattern(char const (&pat)[S])
{
    return {pat, S - 1};
}

template <size_t Length>
class pattern_segment final : public basic_pattern_segment
{
    array<value_type, Length> bytes_;
    size_type tail_;

    template <size_t... I>
    constexpr pattern_segment(auto bytes, size_type tail, std::index_sequence<I...>)
        : bytes_{static_cast<value_type>(bytes[I])...}
        , tail_(tail)
    {
    }

  public:
    template <typename T>
    constexpr pattern_segment(T *bytes, size_type tail = 0)
        : pattern_segment(bytes, tail, std::make_index_sequence<Length>())
    {
    }

    template <typename T>
    constexpr pattern_segment(std::pair<T *, size_type> info)
        : pattern_segment(info.first, info.second, std::make_index_sequence<Length>())
    {
    }

    pointer begin() const override
    {
        return bytes_.data();
    }

    pointer end() const override
    {
        return begin() + Length;
    }

    size_type tail() const override
    {
        return tail_;
    }

    constexpr size_type length() const
    {
        return Length;
    }

    constexpr size_type abs_length() const
    {
        return length() + tail_;
    }

  protected:
    size_type self_size() const override
    {
        return sizeof(pattern_segment);
    }
};

template <>
class pattern_segment<0> final : public basic_pattern_segment
{
    pointer bytes_;
    size_type length_;
    size_type tail_;

  public:
    template <typename T>
    pattern_segment(T *bytes, size_type length, size_type tail = 0)
        : bytes_(reinterpret_cast<pointer>(bytes))
        , length_(length)
        , tail_(tail)
    {
        // std::copy_n(bytes, length, bytes_.begin());
    }

    pointer begin() const override
    {
        return (bytes_);
    }

    pointer end() const override
    {
        return begin() + length_;
    }

    size_type tail() const override
    {
        return tail_;
    }

    constexpr size_type length() const
    {
        return length_;
    }

    constexpr size_type abs_length() const
    {
        return length() + tail_;
    }

  protected:
    size_type self_size() const override
    {
        return sizeof(pattern_segment);
    }
};

template <size_t... SegmentLength>
class pattern final : public basic_pattern
{
    static constexpr bool tuple_store_elements_as_is()
    {
        boost::hana::tuple<char, char> tpl(10, 8);
        return &at_c<0>(tpl) + 1 == &at_c<1>(tpl);
    }

    // non-std tuple because std's one store elements back-to-front
    using buffer_type = boost::hana::tuple<pattern_segment<SegmentLength>...>;
    buffer_type buffer_;

  public:
    constexpr pattern(auto... info)
        : buffer_(info...)
    {
        static_assert(tuple_store_elements_as_is());
    }

    iterator begin() const override
    {
        return &at_c<0>(buffer_);
    }

    iterator end() const override
    {
        return &at_c<sizeof...(SegmentLength) - 1>(buffer_) + 1;
        // return reinterpret_cast<segment>(std::next(&buffer_));
    }

    size_t size() const override
    {
        if constexpr (((SegmentLength == 0) || ...))
            return boost::hana::sum<size_t>(boost::hana::transform(buffer_, [](auto &p) { return p.length(); }));
        else
            return (SegmentLength + ...);
    }

    // size_t length() const override
    //{
    //     return (SegmentLength + ...);
    // }

    // size_t abs_length() const override
    //{
    //     return sum(transform(buffer_, []<size_t L>(pattern_segment<L> const &s) { return L /*+ s.tail()*/; }));
    // }
};

// template <size_t... S>
// pattern(std::pair<pattern_string<S>, ptrdiff_t> const &...) -> pattern<S...>;

template <pattern_string Pattern>
constexpr auto make_pattern()
{
    // ReSharper disable once CppInconsistentNaming
#define make_hint() dynamic_pattern(Pattern.buff, std::size(Pattern.buff))

    constexpr auto convert_segments = []<size_t... I>(std::index_sequence<I...>) {
        auto hint = make_hint();
        return pattern<make_hint()[I].length()...>(hint[I].extract()...);
    };

    return convert_segments(std::make_index_sequence<size(make_hint())>());

#undef make_hint
}

namespace detail
{
template <size_t S>
struct pattern_segment_info
{
    using pointer   = basic_pattern_segment::pointer;
    using size_type = basic_pattern_segment::size_type;

    pointer bytes;
    static constexpr size_type length = S - 1;
    size_type tail;

    pattern_segment_info(char const (&bytes)[S], size_type tail = 0)
        : bytes(reinterpret_cast<pointer>(bytes))
        , tail(tail)
    {
    }

    pattern_segment<S - 1> get() const
    {
        return {bytes, tail};
    }
};

template <>
struct pattern_segment_info<0>
{
    using pointer   = basic_pattern_segment::pointer;
    using size_type = basic_pattern_segment::size_type;

    pointer bytes;
    size_type length;
    size_type tail;

    pattern_segment_info(string_view bytes, size_type tail = 0)
        : bytes(reinterpret_cast<pointer>(bytes.data()))
        , length(static_cast<size_type>(bytes.length()))
        , tail(tail)
    {
        assert(bytes.length() < std::numeric_limits<size_type>::max());
    }

    pattern_segment_info(char const *bytes, size_type length, size_type tail = 0)
        : bytes(reinterpret_cast<pointer>(bytes))
        , length(length)
        , tail(tail)
    {
    }

    pattern_segment<0> get() const
    {
        return pattern_segment<0>{(bytes), length, tail};
    }
};

template <size_t S>
pattern_segment_info(char const (&)[S], auto...) -> pattern_segment_info<S>;

// template <size_t S>
pattern_segment_info(std::type_identity_t<string_view>, auto...) -> pattern_segment_info<0>;

template <class T, typename A1>
constexpr auto pattern_args_to_segments(T &&tpl, A1 &arg1)
{
    return boost::hana::append(tpl, pattern_segment_info(arg1));
}

template <class T, typename A1, typename A2, typename... Args>
constexpr auto pattern_args_to_segments(T &&tpl, A1 &arg1, A2 &arg2, Args &...args)
{
    if constexpr (std::integral<A2>)
    {
        auto new_tpl = boost::hana::append(tpl, pattern_segment_info(arg1, arg2));
        if constexpr (sizeof...(Args) == 0)
            return new_tpl;
        else
            return pattern_args_to_segments(new_tpl, args...);
    }
    else
    {
        auto new_tpl = boost::hana::append(tpl, pattern_segment_info(arg1, 0));
        return pattern_args_to_segments(new_tpl, arg2, args...);
    }
}
} // namespace detail

template <typename... T>
constexpr auto make_pattern(T const &...args) requires(sizeof...(T) > 1)
{
    return boost::hana::unpack(
        detail::pattern_args_to_segments(boost::hana::tuple(), args...),
        []<size_t... S>(detail::pattern_segment_info<S>... segment) {
            return pattern<(S ? S - 1 : 0)...>(segment.get()...);
        });
}

template <pattern_string Pattern>
constexpr auto cached_pattern = make_pattern<Pattern>();

inline namespace literals
{
template <pattern_string Pattern>
constexpr auto operator""_pat()
{
    if (std::is_constant_evaluated())
        return make_pattern<Pattern>();
    else
        return cached_pattern<Pattern>;
}
} // namespace literals

} // namespace fd
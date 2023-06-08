#if defined(_DEBUG) && defined(_MSC_VER)
#pragma optimize("gty", on)
#endif

#include "mem_scanner.h"

#include <boost/container/static_vector.hpp>

#include <algorithm>
#include <cassert>
#include <numeric>
#include <ranges>

namespace fd
{
using pbyte = uint8_t *;
using byte  = uint8_t;

#ifdef _DEBUG
template <typename T, size_t S>
struct static_buffer : std::vector<T>
{
    using std::vector<T>::vector;

    ~static_buffer()
    {
        assert(this->size() <= S);
    }
};
#else
template <typename T, size_t S>
using static_buffer = boost::container::static_vector<T, S>;
#endif

struct bytes_range : boost::noncopyable
{
    static_buffer<byte, 32> part;
    byte skip = 0;

    bytes_range() = default;

    bytes_range(bytes_range &&other) noexcept
        : part(std::move(other.part))
        , skip(other.skip)
    {
    }

    bytes_range &operator=(bytes_range &&other) noexcept
    {
        part = std::move(other.part);
        skip = other.skip;
        return *this;
    }

    size_t whole_size() const
    {
        return part.size() + skip;
    }
};

struct bytes_range_unpacked
{
    pbyte data;
    size_t known_size;
    byte skip;
    size_t whole_size;

    bytes_range_unpacked(bytes_range &rng)
        : data(rng.part.data())
        , known_size(rng.part.size())
        , skip(rng.skip)
        , whole_size(rng.whole_size())
    {
    }
};

static byte to_num(char chr)
{
    // maybe slower, but readable
    switch (chr)
    {
    case '0':
        return 0x0;
    case '1':
        return 0x1;
    case '2':
        return 0x2;
    case '3':
        return 0x3;
    case '4':
        return 0x4;
    case '5':
        return 0x5;
    case '6':
        return 0x6;
    case '7':
        return 0x7;
    case '8':
        return 0x8;
    case '9':
        return 0x9;
    case 'a':
    case 'A':
        return 0xA;
    case 'b':
    case 'B':
        return 0xB;
    case 'c':
    case 'C':
        return 0xC;
    case 'd':
    case 'D':
        return 0xD;
    case 'e':
    case 'E':
        return 0xE;
    case 'f':
    case 'F':
        return 0xF;
    default:
#ifdef assert
        assert(0 && "Unsupported character");
#endif
        return -1;
    }
}

#ifdef _DEBUG
template <typename T>
class simple_range
{
    T begin_, end_;

  public:
    simple_range(T begin, T end)
        : begin_(std::move(begin))
        , end_(std::move(end))
    {
    }

    T begin() const
    {
        return begin_;
    }

    T end() const
    {
        return end_;
    }
};

template <typename T>
simple_range(T, T) -> simple_range<T>;
#else
template <typename T>
using simple_range = std::span<T>;
#endif

struct unknown_bytes_range : static_buffer<bytes_range, 8>
{
    size_t bytes_count() const
    {
        size_t ret = 0;
        for (auto &r : *this)
            ret += r.whole_size();
        return ret;
    }

    struct unknown_bytes_range_unpacked unpack();

    unknown_bytes_range(raw_pattern begin, size_t length)
        : unknown_bytes_range((pbyte)(begin), (pbyte)begin + length)
    {
    }

    unknown_bytes_range(pbyte begin, pbyte end)
    {
        auto back = &this->emplace_back();

        auto store = [&](byte num) {
            if (back->skip > 0)
                back = &this->emplace_back();
            back->part.push_back(num);
        };

        auto skip_byte = [&] {
            ++back->skip;
        };

        auto on_1_char = [&](byte c) {
            if (c == '?')
                skip_byte();
            else
                store(to_num(c));
        };

        auto on_2_chars = [&](byte c, byte c2) {
            if (c == '?')
            {
                assert(c2 == '?');
                skip_byte();
            }
            else
            {
                store(to_num(c) * 16 + to_num(c2));
            }
        };

        auto splitted = std::views::lazy_split(simple_range(begin, end), ' ');
        std::ranges::for_each(splitted, [&](auto part) {
            auto it = part.begin();
            switch (std::ranges::distance(part))
            {
            case 1:
                return on_1_char(*it);
            case 2:
                return on_2_chars(*it, *std::next(it));
            default:
                std::unreachable();
            }
        });
    }
};

struct unknown_bytes_range_unpacked : static_buffer<bytes_range_unpacked, 8>
{
    size_t bytes_count;

    unknown_bytes_range_unpacked(unknown_bytes_range &rng)
        : static_buffer(rng.begin(), rng.end())
    {
        assert(!rng.empty());
        bytes_count = std::accumulate<const_iterator>(
            begin(), end(), static_cast<size_t>(0), [](size_t old, const_reference curr) {
                return old + curr.whole_size;
            });
    }
};

unknown_bytes_range_unpacked unknown_bytes_range::unpack()
{
    return *this;
}

class filter_owner : public callback_stop_token
{
    find_filter *callback_;

  public:
    filter_owner(find_filter *filter)
        : callback_(filter)
    {
    }

    bool invoke(void *result)
    {
        return callback_->invoke(result, this);
    }

    filter_owner *self()
    {
        return this;
    }
};

template <bool Rewrap>
static pbyte not_found(void *end)
{
    if constexpr (Rewrap)
        return nullptr;
    else
        return static_cast<pbyte>(end);
}

template <typename T>
concept filter_with_token = requires(T ptr) { ptr->stop_requested(); };

template <typename T>
concept filter_without_token = requires(T ptr) { ptr->have_stop_token(); };

template <typename T>
concept valid_filter = requires(T ptr) { ptr->invoke(nullptr); };

template <bool Rewrap, typename Filter>
static pbyte find_first(pbyte rng_start, pbyte rng_end, byte value, Filter filter)
{
    for (;;)
    {
        auto result = std::find(rng_start, rng_end, value);
        if (result == rng_end)
            return not_found<Rewrap>(rng_end);
        if constexpr (valid_filter<Filter>)
        {
            if (!filter->invoke(result))
                return not_found<Rewrap>(rng_end);
            if constexpr (filter_with_token<Filter>)
                if (!filter->stop_requested())
                {
                    rng_start = (result) + 1;
                    continue;
                }
        }

        return result;
    }
}

// static bool is_pow_2(size_t val)
//{
//     return val % (val - 1) == 0;
// }

static bool compare_except_first(pbyte rng, pbyte part, size_t part_size)
{
    return std::memcmp(rng + 1, part + 1, part_size - 1) == 0;
}

template <bool Rewrap = true, typename Filter>
static pbyte find_whole(pbyte rng_start, pbyte rng_end, pbyte part_start, size_t part_size, Filter filter)
{
    for (;;)
    {
        auto part_start_found = find_first<false>(rng_start, rng_end, *part_start, std::false_type());
        if (part_start_found == rng_end)
            return not_found<Rewrap>(rng_end);
        if (compare_except_first(part_start_found, part_start, part_size))
        {
            if constexpr (valid_filter<Filter>)
            {
                if (!filter->invoke(part_start_found))
                    return not_found<Rewrap>(rng_end);
                if constexpr (filter_with_token<Filter>)
                    if (!filter->stop_requested())
                    {
                        rng_start = part_start_found + part_size;
                        continue;
                    }
            }
            return part_start_found;
        }
        rng_start = part_start_found + 1;
    }
}

template <bool RngEndPreset = false, bool Rewrap = true, typename Filter>
static pbyte do_search(pbyte rng_start, pbyte rng_end, pbyte part_start, size_t part_size, Filter filter)
{
    assert(part_size != 0);
    if constexpr (valid_filter<Filter>)
        assert(filter != nullptr);

    if constexpr (!RngEndPreset)
        rng_end -= part_size;

    return part_size == 1 ? find_first<Rewrap>(rng_start, rng_end, *(part_start), filter)
                          : find_whole<Rewrap>(rng_start, rng_end, part_start, part_size, filter);
}

template <typename Filter>
static pbyte do_search(pbyte rng_start, pbyte rng_end, bytes_range &bytes, Filter filter)
{
    return do_search<true, true>(rng_start, rng_end - bytes.whole_size(), bytes.part.data(), bytes.part.size(), filter);
}

static bool compare_except_first(pbyte checked, bytes_range_unpacked *rng_begin, bytes_range_unpacked *rng_end)
{
    for (checked += rng_begin->whole_size, ++rng_begin; // skip previously checked block
         rng_begin != rng_end;
         checked += rng_begin->whole_size, ++rng_begin)
    {
        if (std::memcmp(checked, rng_begin->data, rng_begin->known_size) != 0)
            return false;
    }

    return true;
}

template <bool RngEndPreset = false, bool Rewrap = true, typename Filter>
static pbyte do_search(pbyte rng_start, pbyte rng_end, unknown_bytes_range_unpacked &unk_bytes, Filter filter)
{
    if constexpr (valid_filter<Filter>)
        assert(filter != nullptr);

    if constexpr (!RngEndPreset)
        rng_end -= unk_bytes.bytes_count;

    auto unk_bytes_start = unk_bytes.data();

    auto invoker =
        [rng_start,
         rng_end,
         unk_bytes_start,
         unk_bytes_end = unk_bytes_start + unk_bytes.size(),
         filter,
         unk_bytes_count = unk_bytes.bytes_count]<bool StartLength1>(std::bool_constant<StartLength1>) mutable {
            for (pbyte first_part_found;;)
            {
#if 1
                if constexpr (StartLength1)
                    first_part_found = find_first<false>(rng_start, rng_end, *unk_bytes_start->data, std::false_type());
                else
                    first_part_found = find_whole<false>(
                        rng_start, rng_end, unk_bytes_start->data, unk_bytes_start->known_size, std::false_type());
#else
                first_part_found = do_search<true, false>(
                    rng_start, rng_end, unk_bytes_start->data, unk_bytes_start->known_size, std::false_type());
#endif
                if (first_part_found == rng_end)
                    return not_found<Rewrap>(rng_end);
                if (compare_except_first(first_part_found, unk_bytes_start, unk_bytes_end))
                {
                    if constexpr (valid_filter<Filter>)
                    {
                        if (!filter->invoke(first_part_found))
                            return not_found<Rewrap>(rng_end);
                        if constexpr (filter_with_token<Filter>)
                            if (!filter->stop_requested())
                            {
                                rng_start = first_part_found + unk_bytes_count;
                                continue;
                            }
                    }
                    return first_part_found;
                }
                rng_start = first_part_found + unk_bytes_start->known_size;
            }
        };

    return unk_bytes_start->known_size == 1 ? invoker(std::true_type()) : invoker(std::false_type());
}

//-----

void *find_pattern(void *begin, void *end, raw_pattern pattern, size_t pattern_length, find_filter *filter)
{
    auto proxy = [begin = static_cast<pbyte>(begin),
                  end   = static_cast<pbyte>(end)](auto &rng, auto filter_wrapped) -> void * {
        return do_search(begin, end, rng, filter_wrapped);
    };
    auto invoker = [&](auto &&rng) {
        if (!filter)
            return proxy(rng, std::false_type());
        if (filter->have_stop_token())
            return proxy(rng, filter_owner(filter).self());
        return proxy(rng, filter);
    };

    auto range = unknown_bytes_range(pattern, pattern_length);
    return range.size() == 1 ? invoker(range[0]) : invoker(range.unpack());
}

static auto do_search_wrap_filter(void *begin, void *end, raw_bytes bytes, size_t length, find_filter *filter)
{
    auto proxy = [begin = static_cast<pbyte>(begin),
                  end   = static_cast<pbyte>(end),
                  bytes = static_cast<pbyte>(remove_const(bytes)),
                  length](auto filter_wrapped) -> void * {
        return do_search(begin, end, bytes, length, filter_wrapped);
    };

    if (!filter)
        return proxy(nullptr);
    if (filter->have_stop_token())
        return proxy(filter_owner(filter).self());
    return proxy(filter);
}

uintptr_t find_xref(void *begin, void *end, uintptr_t &address, find_filter *filter)
{
    return reinterpret_cast<uintptr_t>(do_search_wrap_filter(begin, end, &address, sizeof(uintptr_t), filter));
}

void *find_bytes(void *begin, void *end, raw_bytes bytes, size_t length, find_filter *filter)
{
    return do_search_wrap_filter(begin, end, bytes, length, filter);
}
} // namespace fd
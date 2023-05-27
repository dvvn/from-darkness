#include "magic_cast.h"
#include "mem_scanner.h"

#include <boost/container/static_vector.hpp>

#include <algorithm>
#include <cassert>
#include <ranges>
#include <span>

namespace fd
{

#if 0
static void* _find_block(const uint8_t* mem, size_t mem_size, const uint8_t* rng, size_t rng_size)
{
    if (mem_size < rng_size)
        return nullptr;

    auto limit = mem_size - rng_size;

    if (rng_size == 1)
        return const_cast<void*>(memchr(mem, *rng, limit));

    for (size_t offset = 0; offset <= limit;)
    {
        auto start1 = memchr(mem + offset, rng[0], limit - offset);
        if (start1 == nullptr)
            break;

        if (memcmp(start1, rng, rng_size) == 0)
            return const_cast<void*>(start1);

        offset = std::distance(mem, static_cast<const uint8_t*>(start1)) + 1;
    }

    return nullptr;
}

static auto _find_block(const uint8_t* mem_begin, const uint8_t* mem_end, const uint8_t* rng_begin, const uint8_t* rng_end)
{
    return _find_block(mem_begin, std::distance(mem_begin, mem_end), rng_begin, std::distance(rng_begin, rng_end));
}

static auto _find_block(const uint8_t* mem_begin, const uint8_t* mem_end, const uint8_t* rng_begin, size_t rng_size)
{
    return _find_block(mem_begin, std::distance(mem_begin, mem_end), rng_begin, rng_size);
}

[[maybe_unused]] static auto _find_block(const uint8_t* mem_begin, size_t mem_size, const uint8_t* rng_begin, const uint8_t* rng_end)
{
    return _find_block(mem_begin, mem_size, rng_begin, std::distance(rng_begin, rng_end));
}
#endif

//-----

struct bytes_range
{
    using part_storage = boost::container::static_vector<uint8_t, 32>;

    part_storage part;
    uint8_t skip = 0;

    bytes_range() = default;

    bytes_range(bytes_range const &other) = delete;

    bytes_range(bytes_range &&other) noexcept
        : part(std::move(other.part))
        , skip(other.skip)
    {
    }

    bytes_range &operator=(bytes_range const &other) = delete;

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
    std::span<uint8_t> part;
    uint8_t skip;
    size_t whole_size;

    bytes_range_unpacked(bytes_range &rng)
        : part(rng.part.data(), rng.part.size())
        , skip(rng.skip)
        , whole_size(rng.whole_size())
    {
    }
};

static uint8_t to_num(char chr)
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

template <typename T>
using bytes_range_buffer = boost::container::static_vector<T, 8>;

struct _unknown_bytes_range : bytes_range_buffer<bytes_range>
{
    size_t bytes_count() const
    {
        size_t ret = 0;
        for (auto &r : *this)
            ret += r.whole_size();
        return ret;
    }

    struct _unknown_bytes_range_unpacked unpack();

    _unknown_bytes_range(char const *begin, size_t length)
        : _unknown_bytes_range(begin, begin + length)
    {
    }

    _unknown_bytes_range(char const *begin, char const *end)
    {
        auto back = &this->emplace_back();

        auto store = [&](uint8_t num) {
            if (back->skip > 0)
                back = &this->emplace_back();
            back->part.push_back(num);
        };
        auto store_byte1 = [&](char chr_num) {
            store(to_num(chr_num));
        };
        auto store_byte2 = [&](char part1, char part2) {
            store(to_num(part1) * 16 + to_num(part2));
        };

        auto skip_byte = [&] {
            ++back->skip;
        };

        while (begin < end - 1)
        {
            auto c = *begin;
            if (c == ' ')
            {
                ++begin;
                continue;
            }
            auto c2 = *(begin + 1);
            if (c2 == ' ')
            {
                if (c == '?')
                    skip_byte();
                else
                    store_byte1(c);
            }
            else
            {
                if (c == '?')
                {
                    assert(c2 == '?');
                    skip_byte();
                }
                else
                {
                    store_byte2(c, c2);
                }
            }
            begin += 2;
        }
    }
};

struct _unknown_bytes_range_unpacked : bytes_range_buffer<bytes_range_unpacked>
{
    size_t bytes_count = 0;

    _unknown_bytes_range_unpacked(_unknown_bytes_range &rng)
        : bytes_range_buffer<bytes_range_unpacked>(rng.begin(), rng.end())
    {
        for (auto &r : *this)
            bytes_count += r.whole_size;
    }
};

_unknown_bytes_range_unpacked _unknown_bytes_range::unpack()
{
    return *this;
}

//-----

#if 0
struct bytes_range_unpacked_tiny
{
    bytes_range::part_storage::const_pointer begin, end;
    size_t skip;

    bytes_range_unpacked_tiny(bytes_range const *rng)
        : begin(rng->part.data())
        , end(rng->part.data() + rng->part.size())
        , skip(rng->skip)

    {
    }

    size_t size() const
    {
        return std::distance(begin, end);
    }
};

struct bytes_range_unpacked : bytes_range_unpacked_tiny
{
    size_t bytesCount;

    bytes_range_unpacked(bytes_range const *rng)
        : bytes_range_unpacked_tiny(rng)
        , bytesCount(rng->whole_size())
    {
    }
};
#endif

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
static uint8_t *not_found(uint8_t *end)
{
    if constexpr (Rewrap)
        return nullptr;
    else
        return end;
}

template <typename T>
concept fiter_pointer = requires(T ptr) { static_cast<filter_owner &>(*ptr); };

template <bool Rewrap, typename Filter>
static uint8_t *_find_first(uint8_t *rng_start, uint8_t *rng_end, uint8_t value, Filter filter)
{
    for (;;)
    {
        auto result = std::find<uint8_t *>(rng_start, rng_end, value);
        if (result == rng_end)
            return not_found<Rewrap>(rng_end);
        if constexpr (fiter_pointer<Filter>)
        {
            if (!filter->invoke(result))
                return not_found<Rewrap>(rng_end);
            if (!filter->stop_requested())
            {
                ++rng_start;
                continue;
            }
        }
        return result;
    }
}

static bool is_pow_2(size_t val)
{
    return val % (val - 1) == 0;
}

static bool _compare_except_first(uint8_t *rng, uint8_t *part, size_t part_size)
{
    return std::memcmp(rng + 1, part + 1, part_size - 1) == 0;
}

template <bool RngEndPreset = false, bool Rewrap = true, typename Filter>
static from<uint8_t *> _search(
    to<uint8_t *> rng_start,
    to<uint8_t *> rng_end,
    to<uint8_t *> part_start,
    size_t part_size,
    Filter filter)
{
    assert(part_size != 0);
    if constexpr (fiter_pointer<Filter>)
        assert(filter != nullptr);

    if constexpr (!RngEndPreset)
        rng_end -= part_size;

    if (part_size == 1)
        return _find_first<Rewrap>(rng_start, rng_end, *part_start, filter);

    for (;;)
    {
        auto part_start_found = _find_first<false>(rng_start, rng_end, *part_start, nullptr);
        if (part_start_found == rng_end)
            return not_found<Rewrap>(rng_end);
        size_t compared = 1;
        if (_compare_except_first(part_start_found, part_start, part_size))
        {
            if constexpr (fiter_pointer<Filter>)
            {
                if (!filter->invoke(part_start_found))
                    return not_found<Rewrap>(rng_end);
                if (!filter->stop_requested())
                {
                    compared = part_size;
                    goto _AGAIN;
                }
            }
            return part_start_found;
        }
    _AGAIN:
        rng_start = part_start_found + compared;
    }
}

template <typename Filter>
static from<uint8_t *> _search(to<uint8_t *> rng_start, to<uint8_t *> rng_end, bytes_range &bytes, Filter filter)
{
    return _search<true, true>(rng_start, rng_end - bytes.whole_size(), bytes.part.data(), bytes.part.size(), filter);
}

static void validate_range(uint8_t *rng_start, uint8_t *rng_end, std::iter_difference_t<uint8_t *> length)
{
    assert(std::distance<uint8_t *>(rng_start, rng_end) >= length);
}

static bool compare_except_first(uint8_t *checked, bytes_range_unpacked *rng_begin, bytes_range_unpacked *rng_end)
{
    for (auto it = rng_begin + 1; it != rng_end; ++it)
    {
        if (std::memcmp(checked, it->part.data(), it->part.size()) != 0)
            return false;
        checked += it->whole_size;
    }
    return true;
}

struct bytes_range_unpacked_info
{
    uint8_t *start;
    size_t length;
    size_t abs_length;

    bytes_range_unpacked_info(bytes_range_unpacked &rng)
        : start(rng.part.data())
        , length(rng.part.size())
        , abs_length(rng.whole_size)
    {
    }
};

template <bool RngEndPreset = false, bool Rewrap = true, typename Filter>
static from<uint8_t *> _search(
    to<uint8_t *> rng_start,
    to<uint8_t *> rng_end,
    _unknown_bytes_range_unpacked &unk_bytes,
    Filter filter)
{
    if constexpr (fiter_pointer<Filter>)
        assert(filter != nullptr);

    validate_range(rng_start, rng_end, unk_bytes.bytes_count);

    if constexpr (!RngEndPreset)
        rng_end -= unk_bytes.bytes_count;

    auto unk_bytes0      = bytes_range_unpacked_info(unk_bytes[0]);
    auto unk_bytes_start = unk_bytes.data();
    auto unk_bytes_end   = unk_bytes_start + unk_bytes.size();

    for (;;)
    {
        uint8_t *first_part_found = _search<true, false>(
            rng_start, rng_end, unk_bytes0.start, unk_bytes0.length, nullptr);
        if (first_part_found == rng_end)
            return not_found<Rewrap>(rng_end);
        auto compared = unk_bytes0.abs_length;
        if (compare_except_first(first_part_found + compared, unk_bytes_start, unk_bytes_end))
        {
            if constexpr (fiter_pointer<Filter>)
            {
                if (!filter->invoke(first_part_found))
                    return not_found<Rewrap>(rng_end);
                if (!filter->stop_requested())
                {
                    compared = unk_bytes.bytes_count;
                    goto _AGAIN;
                }
            }
            return first_part_found;
        }
    _AGAIN:
        rng_start = first_part_found + compared;
    }
}

//-----

void *find_pattern(void *begin, void *end, char const *pattern, size_t pattern_length, find_filter *filter)
{
    auto range = _unknown_bytes_range(pattern, pattern_length);
    void *result;
    auto invoker = [begin, end, filter, &result](auto &&rng) {
        if (!filter)
            result = _search(begin, end, rng, nullptr);
        else
            result = _search(begin, end, rng, filter_owner(filter).self());
    };

    if (range.size() == 1)
        invoker(range[0]);
    else
        invoker(range.unpack());

    return result;
}

static auto search_check_filter(void *begin, void *end, void *bytes, size_t length, find_filter *filter)
{
    auto invoker = [=](auto filter_wrapped) {
        return _search(begin, end, bytes, length, filter_wrapped);
    };

    return filter ? invoker(filter_owner(filter).self()) : invoker(nullptr);
}

uintptr_t find_xref(void *begin, void *end, uintptr_t &address, find_filter *filter)
{
    return search_check_filter(begin, end, &address, sizeof(uintptr_t), filter);
}

void *find_bytes(void *begin, void *end, void *bytes, size_t length, find_filter *filter)
{
    return search_check_filter(begin, end, bytes, length, filter);
}
} // namespace fd
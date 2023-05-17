#include "magic_cast.h"
#include "mem_scanner.h"

#include <boost/container/static_vector.hpp>

#include <algorithm>
#include <cassert>
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
        auto store = [&](uint8_t num) {
            auto &back = this->back();
            auto &rng  = back.skip > 0 ? this->emplace_back() : back;
            rng.part.push_back(num);
        };

        auto store_byte1 = [&](char chr_num) {
            store(to_num(chr_num));
        };

        auto store_byte2 = [&](char part1, char part2) {
            store(to_num(part1) * 16 + to_num(part2));
        };

        auto skip_byte = [&] {
            ++this->back().skip;
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

static bool _have_mem_after(bytes_range const &rng, uint8_t *mem_start, uint8_t *mem_end)
{
    if (!rng.skip)
        return true;
    auto limit = std::distance(mem_start + rng.part.size(), mem_end);
    return limit > 0 && rng.skip < limit;
}

static bool _have_mem_after(std::iter_difference_t<uint8_t *> reserved, uint8_t *mem_start, uint8_t *mem_end)
{
    return reserved <= std::distance(mem_start, mem_end);
}

template <bool RngEndPreset = false>
static from<uint8_t *> _search(
    to<uint8_t *> rng_start,
    to<uint8_t *> rng_end,
    to<uint8_t *> part_start,
    size_t part_size)
{
    assert(part_size != 0);

    if constexpr (!RngEndPreset)
        rng_end -= part_size;

    if (part_size == 1)
    {
        auto first_value = std::find<uint8_t *>(rng_start, rng_end, *part_start);
        return first_value == rng_end ? nullptr : first_value;
    }

    do
    {
        auto first_value = std::find<uint8_t *>(rng_start, rng_end, *part_start);
        if (first_value == rng_end)
            break;
        if (std::memcmp(first_value + 1, part_start + 1, (part_size - 1)) == 0)
            return first_value;
        rng_start = first_value + 1;
    }
    while (rng_start <= rng_end);

    return nullptr;
}

static from<uint8_t *> _search(to<uint8_t *> rng_start, to<uint8_t *> rng_end, bytes_range &bytes)
{
    uint8_t *ptr = _search(rng_start, rng_end, bytes.part.data(), bytes.part.size());
    if (!ptr)
        return nullptr;
    if (!_have_mem_after(bytes, ptr, rng_end))
        return nullptr;
    return ptr;
}

static from<uint8_t *> _search(
    to<uint8_t *> rng_start,
    to<uint8_t *> rng_end,
    _unknown_bytes_range_unpacked const &unk_bytes)
{
    assert(std::distance<uint8_t *>(rng_start, rng_end) >= unk_bytes.bytes_count);

    /*auto rng1     = _memory_range(rng.data(), rng.size() - unk_bytes.bytes_count);
    auto rng1_end = rng1.end();*/

    auto abs_rng_end = rng_end;
    rng_end -= unk_bytes.bytes_count;

    auto unk_part0_start      = unk_bytes[0].part.data();
    auto unk_part0_length     = unk_bytes[0].part.size();
    auto unk_part0_abs_length = unk_bytes[0].whole_size;

    auto unk_bytes_end = unk_bytes.data() + unk_bytes.size();
    auto unk_bytes1    = unk_bytes.data() + 1;

    for (;;)
    {
        uint8_t *part_begin = _search<true>(rng_start, rng_end, unk_part0_start, unk_part0_length);
        if (!part_begin)
            return nullptr;

        auto part1_start = part_begin + unk_part0_abs_length;

        auto found      = true;
        auto temp_begin = part1_start;

        for (auto it = unk_bytes1; it != unk_bytes_end; ++it)
        {
            if (std::memcmp(temp_begin, it->part.data(), it->part.size()) == 0)
            {
                temp_begin += it->whole_size;
            }
            else
            {
                found = false;
                break;
            }
        }

        if (found)
        {
            if (!_have_mem_after(unk_bytes.bytes_count, part_begin, rng_end))
                return nullptr;
            return part_begin;
        }

        /*if (part1_start > rng_end) //unreachable
            return nullptr;*/

        rng_start = part1_start;
    }
}

//-----

class unknown_bytes_range_updater
{
    _unknown_bytes_range *bytes_;

  public:
    unknown_bytes_range_updater(_unknown_bytes_range &bytes)
        : bytes_(&bytes)
    {
        assert(bytes.empty());
        bytes.emplace_back();
    }

  private:
    // ReSharper disable once CppMemberFunctionMayBeConst
    void store(uint8_t num)
    {
        auto &back = bytes_->back();
        auto &rng  = back.skip > 0 ? bytes_->emplace_back() : back;
        rng.part.push_back(num);
    }

  public:
    void store_byte(char chr_num)
    {
        store(to_num(chr_num));
    }

    void store_byte(char part1, char part2)
    {
        store(to_num(part1) * 16 + to_num(part2));
    }

    // ReSharper disable once CppMemberFunctionMayBeConst
    void skip_byte()
    {
        ++bytes_->back().skip;
    }
};

void *find_pattern(void *begin, void *end, char const *pattern, size_t pattern_length)
{
    auto range = _unknown_bytes_range(pattern, pattern_length);
    return range.size() == 1 ? _search(begin, end, range[0]) : _search(begin, end, range);
}

void find_pattern(
    void *begin,
    void *end,
    char const *pattern,
    size_t pattern_length,
    basic_find_callback<void *> const &callback)
{
    auto invoker = [&](auto &&rng) {
        for (;;)
        {
            void *ptr = _search(begin, end, rng);
            if (!ptr)
                return;
            if (!callback(ptr))
                return;
            begin = ptr;
        }
    };

    auto range = _unknown_bytes_range(pattern, pattern_length);
    if (range.size() == 1)
        invoker(range[0]);
    else
        invoker(range.unpack());
}

uintptr_t find_xref(void *begin, void *end, uintptr_t &address)
{
    return (_search((begin), (end), (&address), sizeof(uintptr_t)));
}

bool find_xref(void *begin, void *end, uintptr_t &address, basic_find_callback<uintptr_t &> const &callback)
{
    for (;;)
    {
        uintptr_t xref = _search(begin, end, (&address), sizeof(uintptr_t));
        if (!xref)
            break;
        if (!callback(xref))
            return true;
        (begin) = to<void *>(xref);
    }
    return false;
}

bool find_bytes(void *begin, void *end, void *bytes, size_t length, basic_find_callback<void *> const &callback)
{
    for (;;)
    {
        void *found = _search(begin, end, bytes, length);
        if (!found)
            break;
        if (!callback(found))
            return true;
        (begin) = found;
    }
    return false;
}
} // namespace fd
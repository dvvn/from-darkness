#include "mem_scanner.h"

#include <boost/container/static_vector.hpp>

#include <algorithm>
#include <cassert>
#include <string>

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

::fd::memory_range_unpacked _memory_range::unpack() const
{
    return { begin_, begin_ + length_, length_ };
}

bool _memory_range::update(pointer curr, size_t offset)
{
    auto new_from = curr + offset;
    auto from     = begin_;

    if (from == new_from)
        return false;

    auto to = begin_ + length_;

    assert(new_from > from);
    assert(new_from <= to - offset);

    begin_  = new_from;
    length_ = to - new_from;

    return true;
}

bool _memory_range::update(abstract_pointer curr, size_t offset)
{
    return update(static_cast<pointer>(curr), offset);
}

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
    _memory_range part;
    uint8_t skip;
    size_t whole_size;

    bytes_range_unpacked(bytes_range &rng)
        : part(rng.part.data(), rng.part.size())
        , skip(rng.skip)
        , whole_size(rng.whole_size())
    {
    }
};

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

_pattern_updater_unknown::~_pattern_updater_unknown()
{
    delete bytes_;
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

template <typename P>
static bool _have_mem_after(bytes_range const &rng, P mem_start, P mem_end)
{
    if (!rng.skip)
        return true;
    auto limit = std::distance(mem_start + rng.part.size(), mem_end);
    return limit > 0 && rng.skip < limit;
}

template <typename P>
static bool _have_mem_after(std::iter_difference_t<uint8_t *> reserved, P mem_start, P mem_end)
{
    return reserved <= std::distance(mem_start, mem_end);
}

template <class R, class P>
static R *_search(R *rng_start, R *rng_end, P *part_start, size_t part_size)
{
    using rng_val_t  = std::decay_t<R>;
    using part_val_t = std::decay_t<P>;

    static_assert(sizeof(rng_val_t) == sizeof(part_val_t));

    if (part_size == 1)
    {
        auto first_value = std::find(rng_start, rng_end, *part_start);
        return first_value == rng_end ? nullptr : first_value;
    }

    assert(part_size != 0);

    do
    {
        auto first_value = std::find(rng_start, rng_end, *part_start);
        if (first_value == rng_end)
            break;
        if (std::memcmp(first_value + 1, part_start + 1, (part_size - 1) * sizeof(part_val_t)) == 0)
            return first_value;
        rng_start = first_value + 1;
    }
    while (rng_start <= rng_end);

    return nullptr;
}

template <bool RngEndPreset, class R, class P>
static auto _search(R &rng, P *part_start, size_t part_size)
{
    auto rng_start = rng.data();
    auto rng_end   = rng_start + rng.size();
    if constexpr (!RngEndPreset)
        rng_end -= part_size;

    return _search(rng_start, rng_end, part_start, part_size);
}

template <bool RngEndPreset, class P>
static auto _search(_memory_range rng, P &part)
{
    auto part_size = part.size();

    auto rng_start = rng.begin();
    auto rng_end   = (RngEndPreset ? rng.end() : rng_start + rng.size() - part_size);

    auto part_start = part.data();
    // auto part_end   = part.data() + part_size;

    return _search(rng_start, rng_end, part_start, part_size);
}

static _memory_range::iterator _find_memory_range_small(_memory_range rng, bytes_range &bytes)
{
    auto ptr = _search<false>(rng, bytes.part);
    if (!ptr)
        return nullptr;
    if (!_have_mem_after(bytes, ptr, rng.end()))
        return nullptr;
    return ptr;
}

static _memory_range::iterator _find_memory_range_full(_memory_range rng, _unknown_bytes_range_unpacked unk_bytes)
{
    assert(rng.size() >= unk_bytes.bytes_count);

    auto rng_end = rng.end();

    auto rng1     = _memory_range(rng.begin(), rng.size() - unk_bytes.bytes_count);
    auto rng1_end = rng1.end();

    auto unk_bytes_end = unk_bytes.data() + unk_bytes.size();
    auto unk_bytes1    = unk_bytes.data() + 1;

    for (;;)
    {
        auto part_begin = _search<true>(rng1, unk_bytes[0].part);
        if (!part_begin)
            return nullptr;

        auto part1_begin = part_begin + unk_bytes[0].whole_size;

        auto found      = true;
        auto temp_begin = part1_begin;

        for (auto it = unk_bytes1; it != unk_bytes_end; ++it)
        {
            if (std::memcmp(temp_begin, it->part.begin(), it->part.size()) == 0)
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
            if (!_have_mem_after(unk_bytes.bytes_count, part_begin, rng1_end))
                return nullptr;
            return part_begin;
        }

        if (part1_begin >= rng_end)
            return nullptr;

        rng1 = { part1_begin, rng_end };
    }
}

static void *_find_memory_range(_memory_range rng, _unknown_bytes_range &unk_bytes)
{
    union
    {
        _memory_range::iterator found;
        void *found_void;
    };

    if (unk_bytes.size() == 1)
        found = _find_memory_range_small(rng, unk_bytes[0]);
    else
        found = _find_memory_range_full(rng, unk_bytes);

    return found_void;
}

template <typename P>
static void *_find_memory_range(_memory_range const &rng, P *mem_begin, size_t mem_size)
{
    return (void *)_search<false>(rng, mem_begin, mem_size);
}

static void *_find_memory_range(_memory_range const &rng, _memory_range const &mem)
{
    return (void *)_search<false>(rng, mem);
}

//-----

static constexpr uint8_t _to_num(char const chr)
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
        assert(0 && "Unsupported character");
        return -1;
    }
}

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
    void store_byte(char const chr_num)
    {
        store(_to_num(chr_num));
    }

    void store_byte(char const part1, char const part2)
    {
        store(_to_num(part1) * 16 + _to_num(part2));
    }

    // ReSharper disable once CppMemberFunctionMayBeConst
    void skip_byte()
    {
        ++bytes_->back().skip;
    }
};

template <typename T>
static void _text_to_bytes(unknown_bytes_range_updater updater, T begin, T end)
{
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
                updater.skip_byte();
            else
                updater.store_byte(c);
        }
        else
        {
            if (c == '?')
            {
                assert(c2 == '?');
                updater.skip_byte();
            }
            else
            {
                updater.store_byte(c, c2);
            }
        }
        begin += 2;
    }
}

template <typename T>
static void _text_to_bytes(_unknown_bytes_range &bytes, T begin, size_t size)
{
    _text_to_bytes(bytes, begin, begin + size);
}

static void _text_to_bytes(_unknown_bytes_range &bytes, _memory_range text_src)
{
    _text_to_bytes(bytes, text_src.begin(), text_src.end());
}

//-----

auto memory_scanner::operator()(abstract_pointer begin, size_t mem_size) const -> finder
{
    return {
        {*this, static_cast<pointer>(begin), mem_size}
    };
}

auto pattern_scanner::operator()(pattern_pointer pattern, size_t length) const -> scanner
{
    return {
        {*this, { pattern, length }}
    };
}

memory_scanner pattern_scanner::raw() const
{
    return { begin(), end() };
}

//-----

_pattern_updater_unknown::_pattern_updater_unknown(_pattern_updater_unknown &&other) noexcept
    : _memory_range(std::move(other))
    , bytes_(std::exchange(other.bytes_, nullptr))
{
}

_pattern_updater_unknown &_pattern_updater_unknown::operator=(_pattern_updater_unknown &&other) noexcept
{
    using std::swap;
    swap<_memory_range>(*this, other);
    swap(bytes_, other.bytes_);
    return *this;
}

_pattern_updater_unknown::_pattern_updater_unknown(_memory_range mem, _memory_range pattern)
    : _memory_range(mem)
    , bytes_(new _unknown_bytes_range())
{
    _text_to_bytes(*bytes_, pattern);
}

auto _pattern_updater_unknown::operator()() const -> patter_pointer
{
    return _find_memory_range(*this, *bytes_);
}

bool _pattern_updater_unknown::update(patter_pointer last_pos)
{
    return _memory_range::update(last_pos, 1);
}

//-----

_xrefs_finder::_xrefs_finder(_memory_range mem_rng, xref_reference addr)
    : _memory_range(mem_rng)
    , xref_(reinterpret_cast<pointer>(&addr))
{
}

auto _xrefs_finder::operator()() const -> xref_value
{
    return reinterpret_cast<xref_value>(_find_memory_range(*this, xref_, sizeof(uintptr_t)));
}

bool _xrefs_finder::update(xref_reference last_pos)
{
    return _memory_range::update(reinterpret_cast<abstract_pointer>(last_pos), 1);
}

//-----

void _memory_iterator_dbg_creator::validate(void *other) const
{
    (void)other;
    (void)this;
    assert(ptr_ == other);
}

void _memory_iterator_dbg_creator::validate(_memory_iterator_dbg_creator other) const
{
    (void)other;
    (void)this;
    assert(ptr_ == other.ptr_);
}

//-----

_pattern_updater_known::_pattern_updater_known(_memory_range mem_rng, abstract_pointer begin, size_t mem_size)
    : _memory_range(mem_rng)
    , search_rng_(begin, mem_size)
{
}

auto _pattern_updater_known::operator()() const -> patter_pointer
{
    return _find_memory_range(*this, search_rng_);
}

bool _pattern_updater_known::update(patter_pointer last_pos)
{
    return _memory_range::update(last_pos, search_rng_.size());
}
} // namespace fd
#include "mem_scanner.h"

#include <boost/container/small_vector.hpp>
#ifdef _DEBUG
#include <boost/type_index.hpp>
#include <spdlog/spdlog.h>
#endif

#include <algorithm>
#include <string>

#ifdef _DEBUG
template <>
struct ::fmt::formatter<std::type_info> : formatter<string_view>
{
    template <class Ctx>
    auto format(boost::typeindex::type_index info, Ctx &ctx) const
    {
        return formatter<string_view>::format(info.pretty_name(), ctx);
    }
};

template <class T>
static void _buffer_size_notification(T &buffer, std::type_info const &name)
{
    static size_t maxBuffer_size = 0;
    if (maxBuffer_size < buffer.size())
    {
        maxBuffer_size = buffer.size();
        spdlog::debug("{}: buffer size set to {}", name, maxBuffer_size);
    }
}
#endif

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

bool _memory_range::update(uint8_t const *curr, size_t offset)
{
    auto new_from = curr + offset;
    auto from     = data();

    if (from == new_from)
        return false;

    auto to = from + size();

    assert(new_from > from);
    assert(new_from <= to - offset);

    std::construct_at<std::span<uint8_t const>>(this, new_from, to);
    return true;
}

bool _memory_range::update(void const *curr, size_t offset)
{
    return update(static_cast<uint8_t const *>(curr), offset);
}

using bytes_range_part = boost::container::small_vector<uint8_t, 6>;

struct bytes_range
{
    bytes_range_part part;
    uint8_t skip = 0;

    ~bytes_range()
    {
#ifdef _DEBUG
        _buffer_size_notification(part, typeid(bytes_range));
#endif
        (void)this;
    }

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

class _unknown_bytes_range : public std::vector<bytes_range>
{
    mutable size_t bytes_count_ = 0;

    size_t bytes_count_impl() const
    {
        size_t ret = 0;
        for (auto &[part, skip] : *this)
        {
            ret += part.size();
            ret += skip;
        }
        return ret;
    }

  public:
    size_t bytes_count() const
    {
        if (bytes_count_ == 0)
            bytes_count_ = bytes_count_impl();

        return bytes_count_;
    }
};

_pattern_updater_unknown::~_pattern_updater_unknown()
{
    delete bytes_;
}

//-----

struct memory_range_unpacked
{
    uint8_t const *first;
    uint8_t const *end;
    size_t size;

    memory_range_unpacked(_memory_range const &rng)
        : first(rng.data())
        , end(rng.data() + rng.size())
        , size(rng.size())
    {
    }
};

struct bytes_range_unpacked_tiny
{
    uint8_t const *begin;
    uint8_t const *end;

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

struct unknown_bytes_range_unpacked
{
    // const bytes_range* begin;
    bytes_range const *second;
    bytes_range const *end;

    size_t count;
    size_t bytesCount;

    bytes_range_unpacked firstPart;

    unknown_bytes_range_unpacked(_unknown_bytes_range const &unk_bytes)
        : second(unk_bytes.data() + 1)
        , end(unk_bytes.data() + unk_bytes.size())
        , count(unk_bytes.size())
        , bytesCount(unk_bytes.bytes_count())
        , firstPart(unk_bytes.data())
    {
    }
};

static bool _have_mem_after(bytes_range const &rng, uint8_t const *mem_start, uint8_t const *mem_end)
{
    if (!rng.skip)
        return true;
    auto limit = std::distance(mem_start + rng.part.size(), mem_end);
    return limit > 0 && rng.skip < limit;
}

static bool _have_mem_after(bytes_range_unpacked_tiny const &rng, uint8_t const *mem_start, uint8_t const *mem_end)
{
    if (!rng.skip)
        return true;
    auto limit = std::distance(mem_start + rng.size(), mem_end);
    return limit > 0 && rng.skip < static_cast<size_t>(limit);
}

static bool _have_mem_after(
    std::iter_difference_t<uint8_t *> reserved,
    uint8_t const *mem_start,
    uint8_t const *mem_end)
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
    auto rng_end   = rng.data() + (RngEndPreset ? rng.size() : rng.size() - part_size);

    return _search(rng_start, rng_end, part_start, part_size);
}

template <bool RngEndPreset, class R, class P>
static auto _search(R &rng, P &part)
{
    auto part_size = part.size();

    auto rng_start = rng.data();
    auto rng_end   = rng.data() + (RngEndPreset ? rng.size() : rng.size() - part_size);

    auto part_start = part.data();
    // auto part_end   = part.data() + part_size;

    return _search(rng_start, rng_end, part_start, part_size);
}

static void *_find_memory_range(const _memory_range rng, _unknown_bytes_range const &unk_bytes)
{
    union
    {
        uint8_t const *found;
        void *found_void;
    } part0;

    auto rng_end = rng.data() + rng.size();

    if (unk_bytes.size() == 1)
    {
        part0.found = _search<false>(rng, unk_bytes[0].part);
        if (!part0.found)
            return nullptr;
        if (!_have_mem_after(unk_bytes[0], part0.found, rng_end))
            return nullptr;
        return part0.found_void;
    }

    assert(rng.size() >= unk_bytes.bytes_count());

    auto rng1          = std::span(rng.data(), rng.size() - unk_bytes.bytes_count());
    auto rng1_end      = rng1.data() + rng1.size();
    auto unk_bytes_end = unk_bytes.data() + unk_bytes.size();
    for (;;)
    {
        part0.found = _search<true>(rng1, unk_bytes[0].part);
        if (!part0.found)
            return nullptr;

        auto part1_begin = part0.found + unk_bytes[0].whole_size();

        auto found      = true;
        auto temp_begin = part1_begin;

        for (auto unk_bytes_it = unk_bytes.data() + 1; unk_bytes_it != unk_bytes_end; ++unk_bytes_it)
        {
            if (std::memcmp(temp_begin, unk_bytes_it->part.data(), unk_bytes_it->part.size()) == 0)
            {
                temp_begin += unk_bytes_it->whole_size();
            }
            else
            {
                found = false;
                break;
            }
        }

        if (found)
        {
            if (!_have_mem_after(unk_bytes.bytes_count(), part0.found, rng1_end))
                return nullptr;
            return part0.found_void;
        }

        if (part1_begin >= rng_end)
            return nullptr;

        rng1 = { part1_begin, rng_end };
    }
}

static void *_find_memory_range(_memory_range const &rng, uint8_t const *mem_begin, size_t mem_size)
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
        assert(!"Unsupported character");
        return -1;
    }
}

class unknown_bytes_range_updater
{
    _unknown_bytes_range &bytes_;

  public:
    unknown_bytes_range_updater(_unknown_bytes_range &bytes)
        : bytes_(bytes)
    {
        assert(bytes.empty());
        bytes.emplace_back();
    }

  private:
    // ReSharper disable once CppMemberFunctionMayBeConst
    void store(const uint8_t num)
    {
        auto &back = bytes_.back();
        auto &rng  = back.skip > 0 ? bytes_.emplace_back() : back;
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
        ++bytes_.back().skip;
    }
};

template <typename T>
static void _text_to_bytes(_unknown_bytes_range &bytes, T begin, T end)
{
    auto updater = unknown_bytes_range_updater(bytes);

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

static void _text_to_bytes(_unknown_bytes_range &bytes, std::span<char const> const text_src)
{
    _text_to_bytes(bytes, text_src.begin(), text_src.end());
}

//-----

auto pattern_scanner_raw::operator()(std::span<char const> const sig) const -> finder
{
    return {
        {*this, reinterpret_cast<uint8_t const *>(sig.data()), sig.size()}
    };
}

auto pattern_scanner_raw::operator()(void const *begin, size_t mem_size) const -> finder
{
    return {
        {*this, static_cast<uint8_t const *>(begin), mem_size}
    };
}

auto pattern_scanner_text::operator()(std::span<char const> const sig) const -> finder
{
    return {
        {*this, reinterpret_cast<uint8_t const *>(sig.data()), sig.size()}
    };
}

auto pattern_scanner_text::operator()(void const *begin, size_t mem_size) const -> finder
{
    return {
        {*this, static_cast<uint8_t const *>(begin), mem_size}
    };
}

pattern_scanner_raw pattern_scanner_text::raw() const
{
    return { begin(), end() };
}

auto pattern_scanner::operator()(std::span<char const> const sig) const -> unknown_finder
{
    return {
        {*this, sig}
    };
}

auto pattern_scanner::operator()(uint8_t const *begin, size_t mem_size) const -> known_finder
{
    return {
        {*this, begin, mem_size}
    };
}

pattern_scanner_raw pattern_scanner::raw() const
{
    return { begin(), end() };
}

//-----

_pattern_updater_unknown::_pattern_updater_unknown(_pattern_updater_unknown &&other) noexcept
    : mem_rng_(other.mem_rng_)
    , bytes_(std::exchange(other.bytes_, nullptr))
{
}

_pattern_updater_unknown &_pattern_updater_unknown::operator=(_pattern_updater_unknown &&other) noexcept
{
    using std::swap;
    swap(mem_rng_, other.mem_rng_);
    swap(bytes_, other.bytes_);
    return *this;
}

_pattern_updater_unknown::_pattern_updater_unknown(const _memory_range mem_rng, std::span<char const> const sig)
    : mem_rng_(mem_rng)
    , bytes_(new _unknown_bytes_range())
{
    _text_to_bytes(*bytes_, sig);
}

_pattern_updater_unknown::_pattern_updater_unknown(const _memory_range mem_rng, uint8_t const *begin, size_t mem_size)
    : mem_rng_(mem_rng)
    , bytes_(new _unknown_bytes_range())

{
    _text_to_bytes(*bytes_, begin, mem_size);
}

void *_pattern_updater_unknown::operator()() const
{
    return _find_memory_range(mem_rng_, *bytes_);
}

bool _pattern_updater_unknown::update(void const *last_pos)
{
    return mem_rng_.update(last_pos, 1);
}

//-----

_xrefs_finder::_xrefs_finder(const _memory_range mem_rng, uintptr_t const &addr)
    : mem_rng_(mem_rng)
    , xref_(reinterpret_cast<uint8_t const *>(&addr))
{
}

_xrefs_finder::_xrefs_finder(const _memory_range mem_rng, void const *&addr)
    : mem_rng_(mem_rng)
    , xref_(reinterpret_cast<uint8_t const *>(&addr))
{
}

void *_xrefs_finder::operator()() const
{
    return _find_memory_range(mem_rng_, xref_, sizeof(uintptr_t));
}

bool _xrefs_finder::update(void const *last_pos)
{
    return mem_rng_.update(last_pos, 1);
}

//-----

void _memory_iterator_dbg_creator::validate(void const *other) const
{
    (void)other;
    (void)this;
    assert(ptr_ == other);
}

void _memory_iterator_dbg_creator::validate(const _memory_iterator_dbg_creator other) const
{
    (void)other;
    (void)this;
    assert(ptr_ == other.ptr_);
}

//-----

_pattern_updater_known::_pattern_updater_known(const _memory_range mem_rng, uint8_t const *begin, size_t mem_size)
    : mem_rng_(mem_rng)
    , search_rng_(begin, begin + mem_size)
{
}

void *_pattern_updater_known::operator()() const
{
    return _find_memory_range(mem_rng_, search_rng_);
}

bool _pattern_updater_known::update(void const *last_pos)
{
    return mem_rng_.update(last_pos, search_rng_.size());
}
} // namespace fd
#include "mem_scanner.h"

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
    auto format(boost::typeindex::type_index info, Ctx& ctx) const
    {
        return formatter<string_view>::format(info.pretty_name(), ctx);
    }
};

template <class T>
static void _buffer_size_notification(T& buffer, std::type_info const& name)
{
    static size_t maxBufferSize = 0;
    if (maxBufferSize < buffer.size())
    {
        maxBufferSize = buffer.size();
        spdlog::debug("{}: buffer size set to {}", name, maxBufferSize);
    }
}
#endif

namespace fd
{
#if 0
static void* _find_block(const uint8_t* mem, const size_t memSize, const uint8_t* rng, const size_t rngSize)
{
    if (memSize < rngSize)
        return nullptr;

    const auto limit = memSize - rngSize;

    if (rngSize == 1)
        return const_cast<void*>(memchr(mem, *rng, limit));

    for (size_t offset = 0; offset <= limit;)
    {
        const auto start1 = memchr(mem + offset, rng[0], limit - offset);
        if (start1 == nullptr)
            break;

        if (memcmp(start1, rng, rngSize) == 0)
            return const_cast<void*>(start1);

        offset = std::distance(mem, static_cast<const uint8_t*>(start1)) + 1;
    }

    return nullptr;
}

static auto _find_block(const uint8_t* memBegin, const uint8_t* memEnd, const uint8_t* rngBegin, const uint8_t* rngEnd)
{
    return _find_block(memBegin, std::distance(memBegin, memEnd), rngBegin, std::distance(rngBegin, rngEnd));
}

static auto _find_block(const uint8_t* memBegin, const uint8_t* memEnd, const uint8_t* rngBegin, const size_t rngSize)
{
    return _find_block(memBegin, std::distance(memBegin, memEnd), rngBegin, rngSize);
}

[[maybe_unused]] static auto _find_block(const uint8_t* memBegin, const size_t memSize, const uint8_t* rngBegin, const uint8_t* rngEnd)
{
    return _find_block(memBegin, memSize, rngBegin, std::distance(rngBegin, rngEnd));
}
#endif

//-----

void _memory_range::update(uint8_t const* curr, const size_t offset)
{
    auto newFrom = curr + offset;
    auto from    = data();
    auto to      = from + size();

    if (from == newFrom)
    {
        newFrom = to;
    }
    else
    {
        assert(newFrom > from);
        assert(newFrom <= to - offset);
    }
    std::construct_at<std::span<uint8_t const>>(this, newFrom, to);
}

void _memory_range::update(void const* curr, const size_t offset)
{
    update(static_cast<uint8_t const*>(curr), offset);
}

using bytes_range_part = fmt::basic_memory_buffer<uint8_t, 32>;

struct bytes_range
{
    bytes_range_part part;
    uint8_t          skip = 0;

    ~bytes_range()
    {
#ifdef _DEBUG
        _buffer_size_notification(part, typeid(bytes_range));
#endif
        (void)this;
    }

    bytes_range() = default;

    bytes_range(bytes_range const& other) = delete;

    bytes_range(bytes_range&& other) noexcept
        : part(std::move(other.part))
        , skip(other.skip)
    {
    }

    bytes_range& operator=(bytes_range const& other) = delete;

    bytes_range& operator=(bytes_range&& other) noexcept
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
    mutable size_t bytesCount_ = 0;

    size_t bytes_count_impl() const
    {
        size_t ret = 0;
        for (auto& [part, skip] : *this)
        {
            ret += part.size();
            ret += skip;
        }
        return ret;
    }

  public:
    size_t bytes_count() const
    {
        if (bytesCount_ == 0)
            bytesCount_ = bytes_count_impl();

        return bytesCount_;
    }
};

_pattern_updater_unknown::~_pattern_updater_unknown()
{
    delete bytes_;
}

//-----

struct memory_range_unpacked
{
    uint8_t const* first;
    uint8_t const* end;
    size_t         size;

    memory_range_unpacked(_memory_range const& rng)
        : first(rng.data())
        , end(rng.data() + rng.size())
        , size(rng.size())
    {
    }
};

struct bytes_range_unpacked_tiny
{
    uint8_t const* begin;
    uint8_t const* end;

    size_t skip;

    bytes_range_unpacked_tiny(bytes_range const* rng)
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

    bytes_range_unpacked(bytes_range const* rng)
        : bytes_range_unpacked_tiny(rng)
        , bytesCount(rng->whole_size())
    {
    }
};

struct unknown_bytes_range_unpacked
{
    // const bytes_range* begin;
    bytes_range const* second;
    bytes_range const* end;

    size_t count;
    size_t bytesCount;

    bytes_range_unpacked firstPart;

    unknown_bytes_range_unpacked(_unknown_bytes_range const& unkBytes)
        : second(unkBytes.data() + 1)
        , end(unkBytes.data() + unkBytes.size())
        , count(unkBytes.size())
        , bytesCount(unkBytes.bytes_count())
        , firstPart(unkBytes.data())
    {
    }
};

static bool _have_mem_after(bytes_range const& rng, uint8_t const* memStart, uint8_t const* memEnd)
{
    if (!rng.skip)
        return true;
    auto const limit = std::distance(memStart + rng.part.size(), memEnd);
    return limit > 0 && rng.skip < limit;
}

static bool _have_mem_after(bytes_range_unpacked_tiny const& rng, uint8_t const* memStart, uint8_t const* memEnd)
{
    if (!rng.skip)
        return true;
    auto const limit = std::distance(memStart + rng.size(), memEnd);
    return limit > 0 && rng.skip < static_cast<size_t>(limit);
}

static bool _have_mem_after(
    const std::iter_difference_t<uint8_t*> reserved,
    uint8_t const*                         memStart,
    uint8_t const*                         memEnd)
{
    return reserved <= std::distance(memStart, memEnd);
}

template <class R, class P>
static R* _search(R* rngStart, R* rngEnd, P* partStart, size_t partSize)
{
    using rng_val_t  = std::decay_t<R>;
    using part_val_t = std::decay_t<P>;

    static_assert(sizeof(rng_val_t) == sizeof(part_val_t));

    if (partSize == 1)
    {
        auto firstValue = std::find(rngStart, rngEnd, *partStart);
        return (firstValue == rngEnd) ? nullptr : firstValue;
    }

    assert(partSize != 0);

    do
    {
        auto firstValue = std::find(rngStart, rngEnd, *partStart);
        if (firstValue == rngEnd)
            break;
        if (std::memcmp(firstValue + 1, partStart + 1, (partSize - 1) * sizeof(part_val_t)) == 0)
            return firstValue;
        rngStart = firstValue + 1;
    } while (rngStart <= rngEnd);

    return nullptr;
}

template <bool RngEndPreset, class R, class P>
static auto _search(R& rng, P* partStart, size_t partSize)
{
    auto rngStart = rng.data();
    auto rngEnd   = rng.data() + (RngEndPreset ? rng.size() : rng.size() - partSize);

    return _search(rngStart, rngEnd, partStart, partSize);
}

template <bool RngEndPreset, class R, class P>
static auto _search(R& rng, P& part)
{
    auto partSize = part.size();

    auto rngStart = rng.data();
    auto rngEnd   = rng.data() + (RngEndPreset ? rng.size() : rng.size() - partSize);

    auto partStart = part.data();
    // auto partEnd   = part.data() + partSize;

    return _search(rngStart, rngEnd, partStart, partSize);
}

static void* _find_memory_range(const _memory_range rng, _unknown_bytes_range const& unkBytes)
{
    union
    {
        uint8_t const* found;
        void*          foundVoid;
    } part0;

    auto const rngEnd = rng.data() + rng.size();

    if (unkBytes.size() == 1)
    {
        part0.found = _search<false>(rng, unkBytes[0].part);
        if (!part0.found)
            return nullptr;
        if (!_have_mem_after(unkBytes[0], part0.found, rngEnd))
            return nullptr;
        return part0.foundVoid;
    }

    assert((rng.size()) >= unkBytes.bytes_count());

    auto       rng1        = std::span(rng.data(), rng.size() - unkBytes.bytes_count());
    auto const rng1End     = rng1.data() + rng1.size();
    auto const unkBytesEnd = unkBytes.data() + unkBytes.size();
    for (;;)
    {
        part0.found = _search<true>(rng1, unkBytes[0].part);
        if (!part0.found)
            return nullptr;

        auto const part1Begin = part0.found + unkBytes[0].whole_size();

        auto found     = true;
        auto tempBegin = part1Begin;

        for (auto itUnkBytes = unkBytes.data() + 1; itUnkBytes != unkBytesEnd; ++itUnkBytes)
        {
            if (std::memcmp(tempBegin, itUnkBytes->part.data(), itUnkBytes->part.size()) == 0)
            {
                tempBegin += itUnkBytes->whole_size();
            }
            else
            {
                found = false;
                break;
            }
        }

        if (found)
        {
            if (!_have_mem_after(unkBytes.bytes_count(), part0.found, rng1End))
                return nullptr;
            return part0.foundVoid;
        }

        if (part1Begin >= rngEnd)
            return nullptr;

        rng1 = { part1Begin, rngEnd };
    }
}

static void* _find_memory_range(_memory_range const& rng, uint8_t const* memBegin, const size_t memSize)
{
    return (void*)_search<false>(rng, memBegin, memSize);
}

static void* _find_memory_range(_memory_range const& rng, _memory_range const& mem)
{
    return (void*)_search<false>(rng, mem);
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
        assert("Unsupported character");
        return -1;
    }
}

class unknown_bytes_range_updater
{
    _unknown_bytes_range& bytes_;

  public:
    unknown_bytes_range_updater(_unknown_bytes_range& bytes)
        : bytes_(bytes)
    {
        assert(bytes.empty());
        bytes.emplace_back();
    }

  private:
    // ReSharper disable once CppMemberFunctionMayBeConst
    void store(const uint8_t num)
    {
        auto& back = bytes_.back();
        auto& rng  = back.skip > 0 ? bytes_.emplace_back() : back;
        rng.part.push_back(num);
    }

  public:
    void store_byte(char const chrNum)
    {
        store(_to_num(chrNum));
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
static void _text_to_bytes(_unknown_bytes_range& bytes, T begin, T end)
{
    auto updater = unknown_bytes_range_updater(bytes);

    while (begin < end - 1)
    {
        auto const c = *begin;
        if (c == ' ')
        {
            ++begin;
            continue;
        }
        auto const c2 = *(begin + 1);
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
static void _text_to_bytes(_unknown_bytes_range& bytes, T begin, size_t size)
{
    _text_to_bytes(bytes, begin, begin + size);
}

static void _text_to_bytes(_unknown_bytes_range& bytes, std::span<char const> const textSrc)
{
    _text_to_bytes(bytes, textSrc.begin(), textSrc.end());
}

//-----

auto pattern_scanner_raw::operator()(std::span<char const> const sig) const -> finder
{
    return {
        {*this, reinterpret_cast<uint8_t const*>(sig.data()), sig.size()}
    };
}

auto pattern_scanner_raw::operator()(void const* begin, const size_t memSize) const -> finder
{
    return {
        {*this, static_cast<uint8_t const*>(begin), memSize}
    };
}

auto pattern_scanner_text::operator()(std::span<char const> const sig) const -> finder
{
    return {
        {*this, reinterpret_cast<uint8_t const*>(sig.data()), sig.size()}
    };
}

auto pattern_scanner_text::operator()(void const* begin, size_t memSize) const -> finder
{
    return {
        {*this, static_cast<uint8_t const*>(begin), memSize}
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

auto pattern_scanner::operator()(uint8_t const* begin, const size_t memSize) const -> known_finder
{
    return {
        {*this, begin, memSize}
    };
}

pattern_scanner_raw pattern_scanner::raw() const
{
    return { begin(), end() };
}

//-----

_pattern_updater_unknown::_pattern_updater_unknown(_pattern_updater_unknown&& other) noexcept
    : memRng_(other.memRng_)
    , bytes_(std::exchange(other.bytes_, nullptr))
{
}

_pattern_updater_unknown& _pattern_updater_unknown::operator=(_pattern_updater_unknown&& other) noexcept
{
    using std::swap;
    swap(memRng_, other.memRng_);
    swap(bytes_, other.bytes_);
    return *this;
}

_pattern_updater_unknown::_pattern_updater_unknown(const _memory_range memRng, std::span<char const> const sig)
    : memRng_(memRng)
    , bytes_(new _unknown_bytes_range())
{
    _text_to_bytes(*bytes_, sig);
}

_pattern_updater_unknown::_pattern_updater_unknown(
    const _memory_range memRng,
    uint8_t const*      begin,
    const size_t        memSize)
    : memRng_(memRng)
    , bytes_(new _unknown_bytes_range())

{
    _text_to_bytes(*bytes_, begin, memSize);
}

void* _pattern_updater_unknown::operator()() const
{
    return _find_memory_range(memRng_, *bytes_);
}

void _pattern_updater_unknown::update(void const* lastPos)
{
    memRng_.update(lastPos);
}

//-----

_xrefs_finder::_xrefs_finder(const _memory_range memRng, uintptr_t const& addr)
    : memRng_(memRng)
    , xref_(reinterpret_cast<uint8_t const*>(&addr))
{
}

_xrefs_finder::_xrefs_finder(const _memory_range memRng, void const*& addr)
    : memRng_(memRng)
    , xref_(reinterpret_cast<uint8_t const*>(&addr))
{
}

void* _xrefs_finder::operator()() const
{
    return _find_memory_range(memRng_, xref_, sizeof(uintptr_t));
}

void _xrefs_finder::update(void const* lastPos)
{
    memRng_.update(lastPos);
}

//-----

void _memory_iterator_dbg_creator::validate(void const* other) const
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

_pattern_updater_known::_pattern_updater_known(const _memory_range memRng, uint8_t const* begin, const size_t memSize)
    : memRng_(memRng)
    , searchRng_(begin, begin + memSize)
{
}

void* _pattern_updater_known::operator()() const
{
    return _find_memory_range(memRng_, searchRng_);
}

void _pattern_updater_known::update(void const* lastPos)
{
    memRng_.update(lastPos);
}
} // namespace fd
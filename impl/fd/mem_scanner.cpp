#include <fd/algorithm.h>
#include <fd/assert.h>
#include <fd/mem_scanner.h>

#include <string>
#include <vector>

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

#ifdef _DEBUG
_memory_range::_memory_range(const uint8_t* from, const uint8_t* to)
    : range_view(from, to)
{
    FD_ASSERT(from < to);
}

_memory_range::_memory_range(const uint8_t* from, const size_t size)
    : range_view(from, size)
{
    FD_ASSERT(from < end());
}

_memory_range::_memory_range(range_view rng)
    : range_view(rng.begin(), rng.end())
{
    FD_ASSERT(begin() < end());
}
#endif

void _memory_range::update(const uint8_t* curr, const size_t offset)
{
    auto newFrom = curr + offset;
    auto from    = begin();
    auto to      = end();

    if (from == newFrom)
    {
        newFrom = to;
    }
    else
    {
        FD_ASSERT(newFrom > from);
        FD_ASSERT(newFrom <= to - offset);
    }
    std::construct_at<range_view>(this, newFrom, to);
}

void _memory_range::update(const void* curr, const size_t offset)
{
    update(static_cast<const uint8_t*>(curr), offset);
}

//-----

using bytes_range_part = std::/*std*/ basic_string<uint8_t>;

struct bytes_range
{
    bytes_range_part part;
    uint8_t          skip = 0;

    size_t whole_size() const
    {
        return part.size() + skip;
    }
};

#ifdef _DEBUG
template <>
class range_view<const bytes_range*>
{
    const bytes_range *begin_, *end_;

    struct value_unwrapped
    {
        range_view<const uint8_t*> part;
        uint8_t                    skip;
    };

    class iterator
    {
        const bytes_range* pos_;

      public:
        iterator(const bytes_range* pos)
            : pos_(pos)
        {
        }

        iterator& operator++()
        {
            ++pos_;
            return *this;
        }

        value_unwrapped operator*() const
        {
            return { pos_->part, pos_->skip };
        }

        bool operator==(const iterator& other) const = default;
    };

  public:
    template <class C>
    range_view(C& container)
        : begin_(_begin(container))
        , end_(_end(container))
    {
    }

    range_view(const bytes_range* begin, const bytes_range* end)
        : begin_(begin)
        , end_(end)
    {
    }

    iterator begin() const
    {
        return begin_;
    }

    iterator end() const
    {
        return begin_;
    }
};

// #define _begin std::begin
// #define _end std::end
// #define _size std::size
#endif

class _unknown_bytes_range : public std::vector<bytes_range>
{
    mutable size_t bytesCount_ = 0;

    size_t bytes_count_impl() const
    {
        size_t ret = 0;
        for (auto&& [part, skip] : range_view(*this))
        {
            ret += _size(part);
            ret += skip;
        }
        return ret;
    }

  public:
    size_t bytes_count() const
    {
        if (bytesCount_ == 0)
            bytesCount_ = bytes_count_impl();
        else
            FD_ASSERT(bytesCount_ == bytes_count_impl());

        return bytesCount_;
    }
};

struct memory_range_unpacked
{
    const uint8_t* first;
    const uint8_t* end;
    size_t         size;

    memory_range_unpacked(const _memory_range& rng)
        : first(_begin(rng))
        , end(_end(rng))
        , size(_size(rng))
    {
    }
};

struct bytes_range_unpacked_tiny
{
    const uint8_t* begin;
    const uint8_t* end;

    size_t skip;

    bytes_range_unpacked_tiny(const bytes_range* rng)
        : begin(_begin(rng->part))
        , end(_end(rng->part))
        , skip(rng->skip)

    {
    }
};

static auto _size(const bytes_range_unpacked_tiny& rng)
{
    return std::distance(rng.begin, rng.end);
}

struct bytes_range_unpacked : bytes_range_unpacked_tiny
{
    size_t bytesCount;

    bytes_range_unpacked(const bytes_range* rng)
        : bytes_range_unpacked_tiny(rng)
        , bytesCount(rng->whole_size())
    {
    }
};

struct unknown_bytes_range_unpacked
{
    // const bytes_range* begin;
    const bytes_range* second;
    const bytes_range* end;

    size_t count;
    size_t bytesCount;

    bytes_range_unpacked firstPart;

    unknown_bytes_range_unpacked(const _unknown_bytes_range& unkBytes)
        : second(_begin(unkBytes) + 1)
        , end(_end(unkBytes))
        , count(_size(unkBytes))
        , bytesCount(unkBytes.bytes_count())
        , firstPart(_begin(unkBytes))
    {
    }
};

[[maybe_unused]] static bool _have_mem_after(const bytes_range& rng, const uint8_t* memStart, const uint8_t* memEnd)
{
    if (!rng.skip)
        return true;
    const auto limit = std::distance(memStart + _size(rng.part), memEnd);
    return limit > 0 && rng.skip < limit;
}

static bool _have_mem_after(const bytes_range_unpacked_tiny& rng, const uint8_t* memStart, const uint8_t* memEnd)
{
    if (!rng.skip)
        return true;
    const auto limit = std::distance(memStart + _size(rng), memEnd);
    return limit > 0 && rng.skip < static_cast<size_t>(limit);
}

static bool _have_mem_after(const std::iter_difference_t<uint8_t*> reserved, const uint8_t* memStart, const uint8_t* memEnd)
{
    return reserved <= std::distance(memStart, memEnd);
}

static void* _find_memory_range(_memory_range rng, const _unknown_bytes_range& unkBytes)
{
    union
    {
        const uint8_t* found;
        void*          foundVoid;
    } part0;

    rng = { rng.begin(), rng.end() - unkBytes.bytes_count() + 1 };

    if (unkBytes.size() == 1)
    {
        part0.found = find(rng, unkBytes[0].part);
        if (part0.found == rng.end())
            return nullptr;
        if (!_have_mem_after(unkBytes[0], part0.found, rng.end()))
            return nullptr;
        return part0.foundVoid;
    }

    FD_ASSERT(_size(rng) >= unkBytes.bytes_count());

    for (;;)
    {
        part0.found = find(rng, unkBytes[0].part);
        if (part0.found == rng.end())
            return nullptr;

        const auto part1Begin = part0.found + unkBytes[0].whole_size();

        auto found      = true;
        auto tempBegin  = part1Begin;
        auto itUnkBytes = &unkBytes[1];
        for (; itUnkBytes != _end(unkBytes); ++itUnkBytes)
        {
            if (equal(itUnkBytes->part, tempBegin))
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
            if (!_have_mem_after(unkBytes.bytes_count(), part0.found, rng.end()))
                return nullptr;
            return part0.foundVoid;
        }

        if (part1Begin >= rng.end())
            return nullptr;

        rng = { part1Begin, rng.end() };
    }
}

static void* _find_memory_range(const _memory_range& rng, const uint8_t* memBegin, const size_t memSize)
{
    return (void*)find(rng, memBegin, memSize);
}

static void* _find_memory_range(const _memory_range& rng, const _memory_range& mem)
{
    return (void*)find(rng, mem);
}

//-----

static constexpr uint8_t _to_num(const char chr)
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
        FD_ASSERT_PANIC("Unsupported character");
    }
};

class unknown_bytes_range_updater
{
    _unknown_bytes_range& bytes_;

  public:
    unknown_bytes_range_updater(_unknown_bytes_range& bytes)
        : bytes_(bytes)
    {
        FD_ASSERT(bytes.empty());
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
    void store_byte(const char chrNum)
    {
        store(_to_num(chrNum));
    }

    void store_byte(const char part1, const char part2)
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
    unknown_bytes_range_updater updater(bytes);

    while (begin < end - 1)
    {
        const auto c = *begin;
        if (c == ' ')
        {
            ++begin;
            continue;
        }
        const auto c2 = *(begin + 1);
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
                FD_ASSERT(c2 == '?');
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

template <>
class range_view<const char*> : public range_view<const uint8_t*> // hehe
{
    using range_view<const uint8_t*>::range_view;
};

static size_t _size(const range_view<const char*> rng)
{
    return std::distance(rng.begin(), rng.end());
}

static void _text_to_bytes(_unknown_bytes_range& bytes, const range_view<const char*> textSrc)
{
    _text_to_bytes(bytes, _begin(textSrc), _size(textSrc));
}

//-----

auto pattern_scanner_raw::operator()(const range_view<const char*> sig) const -> finder
{
    return {
        {*this, _begin(sig), _size(sig)}
    };
}

auto pattern_scanner_raw::operator()(const uint8_t* begin, const size_t memSize) const -> finder
{
    return {
        {*this, begin, memSize}
    };
}

auto pattern_scanner_text::operator()(const range_view<const char*> sig) const -> finder
{
    return {
        {*this, _begin(sig), _size(sig)}
    };
}

auto pattern_scanner_text::operator()(const uint8_t* begin, size_t memSize) const -> finder
{
    return {
        {*this, begin, memSize}
    };
}

pattern_scanner_raw pattern_scanner_text::raw() const
{
    return { begin(), end() };
}

auto pattern_scanner::operator()(const range_view<const char*> sig) const -> unknown_finder
{
    return {
        {*this, sig}
    };
}

auto pattern_scanner::operator()(const uint8_t* begin, const size_t memSize) const -> known_finder
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

struct unknown_bytes_range_shared_data
{
    _unknown_bytes_range rng;
    size_t              uses;
};

_unknown_bytes_range_shared::_unknown_bytes_range_shared()
{
    const auto buff = new unknown_bytes_range_shared_data();
    bytes_          = &buff->rng;
    uses_           = &buff->uses;
    // ReSharper disable CppCStyleCast
    FD_ASSERT((void*)buff == (void*)bytes_);
    // ReSharper restore CppCStyleCast
}

_unknown_bytes_range_shared::~_unknown_bytes_range_shared()
{
    if (--*uses_ == 0)
        delete reinterpret_cast<unknown_bytes_range_shared_data*>(bytes_);
}

_unknown_bytes_range_shared::_unknown_bytes_range_shared(const _unknown_bytes_range_shared& other)
{
    *this = other;
}

_unknown_bytes_range_shared& _unknown_bytes_range_shared::operator=(const _unknown_bytes_range_shared& other)
{
    bytes_ = other.bytes_;
    uses_  = other.uses_;
    ++*uses_;
    return *this;
}

_unknown_bytes_range* _unknown_bytes_range_shared::operator->() const
{
    return bytes_;
}

_unknown_bytes_range& _unknown_bytes_range_shared::operator*() const
{
    return *bytes_;
}

_pattern_updater_unknown::_pattern_updater_unknown(const _memory_range memRng, const range_view<const char*> sig)
    : memRng_(memRng)
{
    _text_to_bytes(*bytes_, sig);
}

_pattern_updater_unknown::_pattern_updater_unknown(const _memory_range memRng, const uint8_t* begin, const size_t memSize)
    : memRng_(memRng)
{
    _text_to_bytes(*bytes_, begin, memSize);
}

void* _pattern_updater_unknown::operator()() const
{
    return _find_memory_range(memRng_, *bytes_);
}

void _pattern_updater_unknown::update(const void* lastPos)
{
    memRng_.update(lastPos);
}

//-----

_xrefs_finder::_xrefs_finder(const _memory_range memRng, const uintptr_t& addr)
    : memRng_(memRng)
    , xref_(reinterpret_cast<const uint8_t*>(&addr))
{
}

_xrefs_finder::_xrefs_finder(const _memory_range memRng, const void*& addr)
    : memRng_(memRng)
    , xref_(reinterpret_cast<const uint8_t*>(&addr))
{
}

void* _xrefs_finder::operator()() const
{
    return _find_memory_range(memRng_, xref_, sizeof(uintptr_t));
}

void _xrefs_finder::update(const void* lastPos)
{
    memRng_.update(lastPos);
}

//-----

void _memory_iterator_dbg_creator::validate(const void* other) const
{
    (void)other;
    (void)this;
    FD_ASSERT(ptr_ == other);
}

void _memory_iterator_dbg_creator::validate(const _memory_iterator_dbg_creator other) const
{
    (void)other;
    (void)this;
    FD_ASSERT(ptr_ == other.ptr_);
}

//-----

_pattern_updater_known::_pattern_updater_known(const _memory_range memRng, const uint8_t* begin, const size_t memSize)
    : memRng_(memRng)
    , searchRng_(begin, memSize)
{
}

void* _pattern_updater_known::operator()() const
{
    return _find_memory_range(memRng_, searchRng_);
}

void _pattern_updater_known::update(const void* lastPos)
{
    memRng_.update(lastPos);
}
} // namespace fd
#include <fd/assert.h>
#include <fd/mem_scanner.h>

#include <fd/views.h>
#include <vector>

namespace fd
{
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

//-----

memory_range::memory_range(const uint8_t* from, const uint8_t* to)
    : from(from)
    , to(to)
{
    FD_ASSERT(from < to);
}

memory_range::memory_range(const uint8_t* from, const size_t size)
    : from(from)
    , to(from + size)
{
    FD_ASSERT(from < to);
}

void memory_range::update(const uint8_t* curr, const size_t offset)
{
    const auto newFrom = curr + offset;
#ifdef _DEBUG
    if (offset == 0)
#endif
        if (from == newFrom)
        {
            from = to;
            return;
        }
    FD_ASSERT(newFrom > from);
    FD_ASSERT(newFrom <= to - offset);
    from = newFrom;
}

void memory_range::update(const void* curr, const size_t offset)
{
    update(static_cast<const uint8_t*>(curr), offset);
}

//-----

struct bytes_range
{
    using part_storage = /*std::vector*/ basic_string<uint8_t>;

    part_storage part;
    uint8_t      skip = 0;
};

class unknown_bytes_range : public std::vector<bytes_range>
{
    mutable size_t bytesCount_ = 0;

    size_t bytes_count_impl() const
    {
        size_t ret = 0;
        for (auto& [part, skip] : forward_view(*this))
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
        else
            FD_ASSERT(bytesCount_ == bytes_count_impl());

        return bytesCount_;
    }
};

static bool _have_mem_after(const size_t skip, const void* begin, const size_t offset, const uint8_t* end)
{
    if (skip == 0)
        return true;
    const size_t memAfter = std::distance(static_cast<const uint8_t*>(begin) + offset, end);
    return memAfter >= skip;
}

static void* _find_unk_block(const uint8_t* memBegin, const uint8_t* memEnd, const unknown_bytes_range& unkBytes)
{
    const auto unkBytes0 = unkBytes.data();

    const auto unkPart0     = unkBytes0->part.data();
    const auto unkPart0Size = unkBytes0->part.size();
    const auto unkPart0Skip = unkBytes0->skip;

    if (unkBytes.size() == 1)
    {
        const auto part0Found = _find_block(memBegin, memEnd, unkPart0, unkPart0Size);
        if (part0Found == nullptr)
            return nullptr;
        if (!_have_mem_after(unkPart0Skip, part0Found, unkPart0Size, memEnd))
            return nullptr;
        return part0Found;
    }

    const auto   unkBytesCount = unkBytes.bytes_count();
    const size_t memSize       = std::distance(memBegin, memEnd);
    FD_ASSERT(memSize >= unkBytesCount);

    const auto unkBytes1   = unkBytes0 + 1;
    const auto unkBytesEnd = unkBytes0 + unkBytes.size();

    const auto lastReadablePos = memSize - unkBytesCount;
    for (size_t pos = 0; pos <= lastReadablePos;)
    {
        const auto part0Found = _find_block(memBegin + pos, memEnd, unkPart0, unkPart0Size);
        if (part0Found == nullptr)
            break;

        auto tempBegin = static_cast<const uint8_t*>(part0Found) + unkPart0Size + unkPart0Skip;
        auto found     = true;
        for (auto& [part, skip] : range_view(unkBytes1, unkBytesEnd))
        {
            const auto partSize  = part.size();
            const auto partFound = memcmp(tempBegin, part.data(), partSize) == 0;
            if (partFound)
            {
                tempBegin += partSize + skip;
            }
            else
            {
                found = false;
                break;
            }
        }

        if (found)
        {
            if (!_have_mem_after(unkBytes.back().skip, part0Found, unkBytesCount, memEnd))
                break;
            return part0Found;
        }

        pos = std::distance(memBegin, tempBegin) + 1;
    }

    return nullptr;
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
    unknown_bytes_range& bytes_;

  public:
    unknown_bytes_range_updater(unknown_bytes_range& bytes)
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
static void _text_to_bytes(unknown_bytes_range& bytes, T begin, T end)
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
static void _text_to_bytes(unknown_bytes_range& bytes, T begin, size_t size)
{
    _text_to_bytes(bytes, begin, begin + size);
}

static void _text_to_bytes(unknown_bytes_range& bytes, const string_view textSrc)
{
    _text_to_bytes(bytes, textSrc.data(), textSrc.size());
}

//-----

auto pattern_scanner_raw::operator()(const string_view sig) const -> iterator
{
    return {
        this, {*this, reinterpret_cast<const uint8_t*>(sig.data()), sig.size()}
    };
}

auto pattern_scanner_raw::operator()(const uint8_t* begin, const size_t memSize) const -> iterator
{
    return {
        this, {*this, begin, memSize}
    };
}

auto pattern_scanner_text::operator()(const string_view sig) const -> iterator
{
    return {
        this, {*this, reinterpret_cast<const uint8_t*>(sig.data()), sig.size()}
    };
}

auto pattern_scanner_text::operator()(const uint8_t* begin, size_t memSize) const -> iterator
{
    return {
        this, {*this, begin, memSize}
    };
}

pattern_scanner_raw pattern_scanner_text::raw() const
{
    return { from, to };
}

auto pattern_scanner::operator()(const string_view sig) const -> unknown_iterator
{
    return {
        this, {*this, sig}
    };
}

auto pattern_scanner::operator()(const uint8_t* begin, const size_t memSize) const -> known_iterator
{
    return {
        this, {*this, begin, memSize}
    };
}

pattern_scanner_raw pattern_scanner::raw() const
{
    return { from, to };
}

//-----

struct unknown_bytes_range_shared_data
{
    unknown_bytes_range rng;
    size_t              uses;
};

unknown_bytes_range_shared::unknown_bytes_range_shared()
{
    const auto buff = new unknown_bytes_range_shared_data();
    bytes_          = &buff->rng;
    uses_           = &buff->uses;
    // ReSharper disable CppCStyleCast
    FD_ASSERT((void*)buff == (void*)bytes_);
    // ReSharper restore CppCStyleCast
}

unknown_bytes_range_shared::~unknown_bytes_range_shared()
{
    if (--*uses_ == 0)
        delete reinterpret_cast<unknown_bytes_range_shared_data*>(bytes_);
}

unknown_bytes_range_shared::unknown_bytes_range_shared(const unknown_bytes_range_shared& other)
{
    *this = other;
}

unknown_bytes_range_shared& unknown_bytes_range_shared::operator=(const unknown_bytes_range_shared& other)
{
    bytes_ = other.bytes_;
    uses_  = other.uses_;
    ++*uses_;
    return *this;
}

unknown_bytes_range* unknown_bytes_range_shared::operator->() const
{
    return bytes_;
}

unknown_bytes_range& unknown_bytes_range_shared::operator*() const
{
    return *bytes_;
}

pattern_updater_unknown::pattern_updater_unknown(const memory_range memRng, const string_view sig)
    : memRng_(memRng)
{
    _text_to_bytes(*bytes_, sig);
}

pattern_updater_unknown::pattern_updater_unknown(const memory_range memRng, const uint8_t* begin, const size_t memSize)
    : memRng_(memRng)
{
    _text_to_bytes(*bytes_, begin, memSize);
}

void* pattern_updater_unknown::operator()() const
{
    return _find_unk_block(memRng_.from, memRng_.to, *bytes_);
}

void pattern_updater_unknown::update(const void* lastPos)
{
    memRng_.update(lastPos);
}

//-----

xrefs_finder_impl::xrefs_finder_impl(const memory_range memRng, const uintptr_t& addr)
    : memRng_(memRng)
    , xref_(reinterpret_cast<const uint8_t*>(&addr))
{
}

xrefs_finder_impl::xrefs_finder_impl(const memory_range memRng, const void*& addr)
    : memRng_(memRng)
    , xref_(reinterpret_cast<const uint8_t*>(&addr))
{
}

void* xrefs_finder_impl::operator()() const
{
    return _find_block(memRng_.from, memRng_.to, xref_, sizeof(uintptr_t));
}

void xrefs_finder_impl::update(const void* lastPos)
{
    memRng_.update(lastPos);
}

//-----

void memory_iterator_dbg_creator::validate(const void* other) const
{
    (void)other;
    FD_ASSERT(ptr_ == other);
}

void memory_iterator_dbg_creator::validate(const memory_iterator_dbg_creator other) const
{
    (void)other;
    FD_ASSERT(ptr_ == other.ptr_);
}

//-----

pattern_updater_known::pattern_updater_known(const memory_range memRng, const uint8_t* begin, const size_t memSize)
    : memRng_(memRng)
    , searchRng_(begin, memSize)
{
}

void* pattern_updater_known::operator()() const
{
    return _find_block(memRng_.from, memRng_.to, searchRng_.from, searchRng_.to);
}

void pattern_updater_known::update(const void* lastPos)
{
    memRng_.update(lastPos);
}
}
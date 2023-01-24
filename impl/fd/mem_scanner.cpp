#include <fd/assert.h>
#include <fd/mem_scanner.h>

#include <ranges>
#include <vector>

using namespace fd;

static void* _find_block(const uint8_t* start0, const size_t blockSize, const uint8_t* start2, const size_t rngSize)
{
    if (blockSize < rngSize)
        return nullptr;

    const auto limit = blockSize - rngSize;

    if (rngSize == 1)
        return const_cast<void*>(std::memchr(start0, *start2, limit));

    for (size_t offset = 0; offset <= limit;)
    {
        const auto start1 = std::memchr(start0 + offset, start2[0], limit - offset);
        if (start1 == nullptr)
            break;

        if (std::memcmp(start1, start2, rngSize) == 0)
            return const_cast<void*>(start1);

        offset = std::distance(start0, static_cast<const uint8_t*>(start1)) + 1;
    }

    return nullptr;
}

static auto _find_block(const uint8_t* start0, const uint8_t* end0, const uint8_t* start2, const uint8_t* end2)
{
    return _find_block(start0, std::distance(start0, end0), start2, std::distance(start2, end2));
}

static auto _find_block(const uint8_t* start0, const uint8_t* end0, const uint8_t* start2, const size_t rngSize)
{
    return _find_block(start0, std::distance(start0, end0), start2, rngSize);
}

[[maybe_unused]] static auto _find_block(const uint8_t* start0, const size_t blockSize, const uint8_t* start2, const uint8_t* end2)
{
    return _find_block(start0, blockSize, start2, std::distance(start2, end2));
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
    std::vector<uint8_t> part;
    uint8_t              skip = 0;
};

class fd::unknown_bytes_range : public std::vector<bytes_range>
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

static void* _find_unk_block(const uint8_t* begin, const uint8_t* end, const unknown_bytes_range& unkBytes)
{
#ifdef _DEBUG
    const auto   unkBytes_count = unkBytes.bytes_count();
    const size_t memSize = std::distance(begin, end);
    FD_ASSERT(memSize >= unkBytes_count);
#endif

    const auto unkBytes_begin = unkBytes.data();
    const auto unkPart0_size = unkBytes_begin->part.size();
    const auto unkPart0_begin = unkBytes_begin->part.data();
    const auto unkPart0_skip = unkBytes_begin->skip;

    if (unkBytes.size() == 1)
    {
        const auto part0_found = _find_block(begin, end, unkPart0_begin, unkPart0_size);
        if (part0_found == nullptr)
            return nullptr;
        if (!_have_mem_after(unkPart0_skip, part0_found, unkPart0_size, end))
            return nullptr;
        return part0_found;
    }

#ifndef _DEBUG
    const auto   unkBytes_count = unkBytes.bytes_count();
    const size_t memSize = std::distance(begin, end);
#endif

    const auto unkBytes1 = unkBytes_begin + 1;
    const auto unkBytes_end = unkBytes_begin + unkBytes.size();
    const auto lastReadablePos = memSize - unkBytes_count;

    for (size_t pos = 0; pos <= lastReadablePos;)
    {
        const auto part0_found = _find_block(begin + pos, end, unkPart0_begin, unkPart0_size);
        if (part0_found == nullptr)
            break;

        auto temp_begin = static_cast<const uint8_t*>(part0_found) + unkPart0_size + unkPart0_skip;
        auto found = true;
        for (auto unkBytes_itr = unkBytes1; unkBytes_itr != unkBytes_end; ++unkBytes_itr)
        {
            const auto& [part, skip] = *unkBytes_itr;
            const auto part_size = part.size();

            const auto part_valid = std::memcmp(temp_begin, part.data(), part_size) == 0;
            if (!part_valid)
            {
                found = false;
                break;
            }
            temp_begin += part_size + skip;
        }

        if (found)
        {
            if (!_have_mem_after(unkBytes.back().skip, part0_found, unkBytes_count, end))
                break;
            return part0_found;
        }

        pos = std::distance(begin, temp_begin) + 1;
    }

    return nullptr;
}

//-----

static constexpr uint8_t _to_num(const char chr)
{
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
    unknown_bytes_range* bytes_;

  public:
    unknown_bytes_range_updater(unknown_bytes_range& bytes)
        : bytes_(&bytes)
    {
        FD_ASSERT(bytes.empty());
        bytes.emplace_back();
    }

    void store_byte(const uint8_t num) const
    {
        auto& back = bytes_->back();
        auto& rng = back.skip > 0 ? bytes_->emplace_back() : back;
        rng.part.push_back(num);
    }

    void store_byte(const std::same_as<char> auto chrNum) const
    {
        store_byte(_to_num(chrNum));
    }

    void store_byte(const char part1, const char part2) const
    {
        store_byte(_to_num(part1) * 16 + _to_num(part2));
    }

    void skip_byte() const
    {
        ++bytes_->back().skip;
    }
};

static void _text_to_bytes(unknown_bytes_range& bytes, const string_view textSrc)
{
    const unknown_bytes_range_updater updater(bytes);

    for (const auto rng : textSrc | std::views::split(' '))
    {
        const auto   rawBegin = std::addressof(*rng.begin());
        const size_t size = std::ranges::distance(rng);

        switch (size)
        {
        case 1: {
            const auto value = *rawBegin;
            if (value == '?')
                updater.skip_byte();
            else
                updater.store_byte(value);
            break;
        }
        case 2: {
            const auto value1 = rawBegin[0];
            const auto value2 = rawBegin[1];
            if (value1 == '?')
            {
                FD_ASSERT(value2 == '?');
                updater.skip_byte();
            }
            else
            {
                updater.store_byte(value1, value2);
            }
            break;
        }
        default: {
            FD_ASSERT_PANIC("Uncorrect part!");
        }
        }
    }
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

//-------------

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
    bytes_ = &buff->rng;
    uses_ = &buff->uses;
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
    uses_ = other.uses_;
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

pattern_scanner_unknown::pattern_scanner_unknown(const memory_range memRng, const string_view sig)
    : memRng_(memRng)
{
    _text_to_bytes(*bytes_, sig);
}

void* pattern_scanner_unknown::operator()() const
{
    return _find_unk_block(memRng_.from, memRng_.to, *bytes_);
}

void pattern_scanner_unknown::update(const void* lastPos)
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
    FD_ASSERT(ptr_ == other);
}

void memory_iterator_dbg_creator::validate(const memory_iterator_dbg_creator other) const
{
    FD_ASSERT(ptr_ == other.ptr_);
}

//-----

pattern_scanner_known::pattern_scanner_known(const memory_range memRng, const uint8_t* begin, const size_t memSize)
    : memRng_(memRng)
    , searchRng_(begin, memSize)
{
}

void* pattern_scanner_known::operator()() const
{
    return _find_block(memRng_.from, memRng_.to, searchRng_.from, searchRng_.to);
}

void pattern_scanner_known::update(const void* lastPos)
{
    memRng_.update(lastPos);
}
module;

#include <fd/assert.h>

#include <algorithm>
#include <limits>
#include <ranges>
#include <vector>

module fd.mem_scanner;

static void* _Find_block(const pointer start0, const size_t block_size, const pointer start2, const size_t rng_size)
{
    const auto limit = block_size - rng_size;

    if (rng_size == 1)
        return const_cast<void*>(std::memchr(start0, *start2, limit));

    for (size_t offset = 0; offset <= limit;)
    {
        const auto start1 = std::memchr(start0 + offset, start2[0], limit - offset);
        if (!start1)
            break;

        if (std::memcmp(start1, start2, rng_size) == 0)
            return const_cast<void*>(start1);

        offset = std::distance(start0, static_cast<pointer>(start1)) + 1;
    }

    return nullptr;
}

static auto _Find_block(const pointer start0, const pointer end0, const pointer start2, const pointer end2)
{
    return _Find_block(start0, std::distance(start0, end0), start2, std::distance(start2, end2));
}

static auto _Find_block(const pointer start0, const pointer end0, const pointer start2, const size_t rng_size)
{
    return _Find_block(start0, std::distance(start0, end0), start2, rng_size);
}

static auto _Find_block(const pointer start0, const size_t block_size, const pointer start2, const pointer end2)
{
    return _Find_block(start0, block_size, start2, std::distance(start2, end2));
}

//-----

memory_range::memory_range(pointer from, pointer to)
    : from_(from)
    , to_(to)
{
    FD_ASSERT(from_ < to_);
}

memory_range::memory_range(pointer from, const size_t size)
    : from_(from)
    , to_(from + size)
{
    FD_ASSERT(from_ < to_);
}

void memory_range::update(pointer curr, const size_t offset)
{
    const auto new_from = curr + offset;
    FD_ASSERT(new_from > from_);
    FD_ASSERT(new_from <= to_ - offset);
    from_ = new_from;
}

void memory_range::update(void* curr, const size_t offset)
{
    update(static_cast<pointer>(curr), offset);
}

//-----

struct bytes_range
{
    std::vector<uint8_t> part;
    uint8_t skip = 0;
};

class unknown_bytes_range : public std::vector<bytes_range>
{
    mutable size_t bytes_count_cached_ = 0;

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
        if (bytes_count_cached_ == 0)
            bytes_count_cached_ = bytes_count_impl();
        else
            FD_ASSERT(bytes_count_cached_ == bytes_count_impl());

        return bytes_count_cached_;
    }
};

static bool _Have_mem_after(const size_t skip, const void* begin, const size_t offset, const pointer end)
{
    if (skip == 0)
        return true;
    const size_t mem_after = std::distance(static_cast<pointer>(begin) + offset, end);
    return mem_after >= skip;
}

static void* _Find_unk_block(const pointer begin, const pointer end, const unknown_bytes_range& unkbytes)
{
#ifdef _DEBUG
    const auto unkbytes_count = unkbytes.bytes_count();
    const size_t mem_size     = std::distance(begin, end);
    FD_ASSERT(mem_size >= unkbytes_count);
#endif

    const auto unkbytes_begin = unkbytes.data();
    const auto unkpart0_size  = unkbytes_begin->part.size();
    const auto unkpart0_begin = unkbytes_begin->part.data();
    const auto unkpart0_skip  = unkbytes_begin->skip;

    if (unkbytes.size() == 1)
    {
        const auto part0_found = _Find_block(begin, end, unkpart0_begin, unkpart0_size);
        if (!part0_found)
            return nullptr;
        if (!_Have_mem_after(unkpart0_skip, part0_found, unkpart0_size, end))
            return nullptr;
        return part0_found;
    }

#ifndef _DEBUG
    const auto unkbytes_count = unkbytes.bytes_count();
    const size_t mem_size     = std::distance(begin, end);
#endif

    const auto unkbytes1         = unkbytes_begin + 1;
    const auto unkbytes_end      = unkbytes_begin + unkbytes.size();
    const auto last_readable_pos = mem_size - unkbytes_count;

    for (size_t pos = 0; pos <= last_readable_pos;)
    {
        const auto part0_found = _Find_block(begin + pos, end, unkpart0_begin, unkpart0_size);
        if (!part0_found)
            break;

        auto temp_begin = static_cast<pointer>(part0_found) + unkpart0_size + unkpart0_skip;
        auto found      = true;
        for (auto unkbytes_itr = unkbytes1; unkbytes_itr != unkbytes_end; ++unkbytes_itr)
        {
            const auto& [part, skip] = *unkbytes_itr;
            const auto part_size     = part.size();

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
            if (!_Have_mem_after(unkbytes.back().skip, part0_found, unkbytes_count, end))
                break;
            return part0_found;
        }

        pos = std::distance(begin, temp_begin) + 1;
    }

    return nullptr;
}

//-----

static void _Text_to_bytes(unknown_bytes_range& bytes, const fd::string_view text_src)
{
    FD_ASSERT(bytes.empty());
    bytes.emplace_back();

    constexpr auto to_num = [](const auto chr) -> uint8_t {
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
            FD_ASSERT_UNREACHABLE("Unsupported character");
        }
    };

    const auto skip_byte = [&] {
        ++bytes.back().skip;
    };

    const auto store_byte = [&](const uint8_t num) {
        auto& back = bytes.back();
        auto& rng  = back.skip > 0 ? bytes.emplace_back() : back;
        rng.part.push_back(num);
    };

    for (const auto rng : text_src | std::views::split(' '))
    {
        const auto raw_begin = std::addressof(*rng.begin());
        const size_t size    = std::ranges::distance(rng);

        switch (size)
        {
        case 1: {
            const auto value = *raw_begin;
            if (value == '?')
                skip_byte();
            else
                store_byte(to_num(value));
            break;
        }
        case 2: {
            const auto value1 = raw_begin[0];
            const auto value2 = raw_begin[1];
            if (value1 == '?' || value2 == '?')
            {
                FD_ASSERT(value1 == value2);
                skip_byte();
            }
            else
            {
                store_byte(to_num(value1) * 16 * to_num(value2));
            }
            break;
        }
        default: {
            FD_ASSERT_UNREACHABLE("Uncorrect part!");
        }
        };
    };
}

//-----

auto pattern_scanner::operator()(const fd::string_view sig) const -> unknown_iterator
{
    return {
        this, {*this, sig}
    };
}

auto pattern_scanner::operator()(const fd::string_view sig, const raw_pattern_t) const -> known_iterator
{
    return {
        this, {*this, reinterpret_cast<pointer>(sig.data()), sig.size()}
    };
}

auto pattern_scanner::operator()(const pointer begin, const size_t mem_size) const -> known_iterator
{
    return {
        this, {*this, begin, mem_size}
    };
}

//-----

struct unknown_bytes_range_shared_data
{
    unknown_bytes_range rng;
    size_t uses;
};

unknown_bytes_range_shared::unknown_bytes_range_shared()
{
    auto buff = new unknown_bytes_range_shared_data();
    bytes_    = &buff->rng;
    uses_     = &buff->uses;
    FD_ASSERT((void*)buff == (void*)bytes_);
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
    ++uses_;
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

pattern_scanner_unknown::pattern_scanner_unknown(const memory_range mem_rng, const fd::string_view sig)
    : mem_rng_(mem_rng)
    , bytes_()
{
    _Text_to_bytes(*bytes_, sig);
}

void* pattern_scanner_unknown::operator()() const
{
    return _Find_unk_block(mem_rng_.from_, mem_rng_.to_, *bytes_);
}

void pattern_scanner_unknown::update(void* last_pos)
{
    mem_rng_.update(last_pos);
}

//-----

xrefs_finder_impl::xrefs_finder_impl(const memory_range mem_rng, const uintptr_t& addr)
    : mem_rng_(mem_rng)
    , xref_(reinterpret_cast<pointer>(&addr))
{
}

xrefs_finder_impl::xrefs_finder_impl(const memory_range mem_rng, const void* addr)
    : mem_rng_(mem_rng)
    , xref_(reinterpret_cast<pointer>(&addr))
{
}

void* xrefs_finder_impl::operator()() const
{
    return _Find_block(mem_rng_.from_, mem_rng_.to_, xref_, sizeof(uintptr_t));
}

void xrefs_finder_impl::update(void* last_pos)
{
    mem_rng_.update(last_pos);
}

//-----

pattern_scanner_known::pattern_scanner_known(const memory_range mem_rng, const pointer begin, const size_t mem_size)
    : mem_rng_(mem_rng)
    , search_rng_(begin, mem_size)
{
}

void* pattern_scanner_known::operator()() const
{
    return _Find_block(mem_rng_.from_, mem_rng_.to_, search_rng_.from_, search_rng_.to_);
}

void pattern_scanner_known::update(void* last_pos)
{
    mem_rng_.update(last_pos);
}

module;

#include <fd/assert.h>

#include <algorithm>
#include <ranges>
#include <vector>

module fd.mem_scanner;

static const void* _Find_block(const pointer start0, const size_t block_size, const pointer start2, const size_t rng_size)
{
    const auto limit = block_size - rng_size;

    if (rng_size == 1)
        return std::memchr(start0, *start2, limit);

    for (size_t offset = 0; offset <= limit;)
    {
        const auto start1 = std::memchr(start0 + offset, start2[0], limit - offset);
        if (!start1)
            break;

        if (std::memcmp(start1, start2, rng_size) == 0)
            return start1;

        offset = std::distance(start0, static_cast<pointer>(start1)) + 1;
    }

    return nullptr;
}

static const void* _Find_block(const pointer start0, const pointer end0, const pointer start2, const pointer end2)
{
    return _Find_block(start0, std::distance(start0, end0), start2, std::distance(start2, end2));
}

static const void* _Find_block(const pointer start0, const pointer end0, const pointer start2, const size_t rng_size)
{
    return _Find_block(start0, std::distance(start0, end0), start2, rng_size);
}

static const void* _Find_block(const pointer start0, const size_t block_size, const pointer start2, const pointer end2)
{
    return _Find_block(start0, block_size, start2, std::distance(start2, end2));
}

//-----

struct bytes_range
{
    std::vector<uint8_t> part;
    uint8_t skip = 0;
};

struct unknown_bytes_range : std::vector<bytes_range>
{
    size_t bytes_count() const
    {
        size_t ret = 0;
        for (auto& [part, skip] : *this)
        {
            ret += part.size();
            ret += skip;
        }
        return ret;
    }
};

static bool _Have_mem_after(const size_t skip, const void* begin, const size_t offset, const pointer end)
{
    if (skip == 0)
        return true;
    const size_t mem_after = std::distance(static_cast<pointer>(begin) + offset, end);
    return mem_after >= skip;
}

static const void* _Find_unk_block(const pointer begin, const pointer end, const unknown_bytes_range& unkbytes)
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

    constexpr auto unwrap_shit = [](auto rng) -> fd::string_view {
        const auto raw_begin = std::addressof(*rng.begin());
        const size_t size    = std::ranges::distance(rng);
        return { raw_begin, size };
    };

    for (const auto b : std::views::split(text_src, ' ') | std::views::transform(unwrap_shit))
    {
        if (b[0] == '?')
        {
            FD_ASSERT(b.size() == 1 || b.size() == 2 && b[1] == '?');
            ++bytes.back().skip;
            continue;
        }

        // take new part
        if (bytes.back().skip > 0)
            bytes.emplace_back();

        const auto num_left = to_num(b[0]);
        if (b.size() == 1)
        {
            bytes.back().part.push_back(num_left);
            continue;
        }
        FD_ASSERT(b.size() == 2);
        const auto num_right = to_num(b[1]);
        const auto num       = num_left * 16 + num_right;
        bytes.back().part.push_back(num);
    }
}

void* memory_scanner::operator()(const fd::string_view sig, const bool raw) const
{
    const void* ret;

    if (raw)
    {
        ret = _Find_block(from_, to_, (pointer)sig.data(), sig.size());
    }
    else
    {
        unknown_bytes_range bytes;
        _Text_to_bytes(bytes, sig);
        ret = _Find_unk_block(from_, to_, bytes);
    }
    return const_cast<void*>(ret);
}

void* memory_scanner::operator()(const pointer begin, const size_t mem_size) const
{
    return const_cast<void*>(_Find_block(from_, to_, begin, mem_size));
}

//-----

unknown_bytes_wrapped::unknown_bytes_wrapped(const fd::string_view sig)
{
    auto bytes_ = new unknown_bytes_range();
    _Text_to_bytes(*bytes_, sig);
}

unknown_bytes_wrapped::~unknown_bytes_wrapped()
{
    delete bytes_;
}

size_t unknown_bytes_wrapped::count() const
{
    return bytes_->bytes_count();
}

void* unknown_bytes_wrapped::find_in(const pointer begin, const pointer end) const
{
    return const_cast<void*>(_Find_unk_block(begin, end, *bytes_));
}

//-----

uintptr_t xrefs_finder::operator()(const uintptr_t addr) const
{
    return reinterpret_cast<uintptr_t>(_Find_block(from_, to_, (pointer)&addr, sizeof(uintptr_t)));
}

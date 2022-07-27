module;

#include <fd/assert.h>

#include <algorithm>
#include <ranges>
#include <vector>

module fd.signature;

static const void* _Find_block(const pointer start0, const pointer end0, const pointer start2, const pointer end2)
{
    const size_t block_size = std::distance(start0, end0);
    const size_t rng_size   = std::distance(start2, end2);
    const auto limit      = block_size - rng_size;

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

static const void* _Find_block(const pointer start0, const pointer end0, const pointer start2, const size_t mem_size)
{
    return _Find_block(start0, end0, start2, start2 + mem_size);
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
        if (unkpart0_skip > 0)
        {
            const auto mem_after = std::distance(static_cast<pointer>(part0_found) + unkpart0_size, end);
            if (mem_after < unkpart0_skip)
                return nullptr;
        }
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

        auto temp_begin = (pointer)part0_found + unkpart0_size + unkpart0_skip;
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

        if (!found)
        {
            pos = std::distance(begin, temp_begin) + 1;
            continue;
        }

        const auto unkbytes_last_skip = unkbytes.back().skip;
        if (unkbytes_last_skip > 0)
        {
            const auto mem_after = std::distance(static_cast<pointer>(part0_found) + unkbytes_count, end);
            if (mem_after < unkbytes_last_skip)
                break;
        }
        return part0_found;
    }

    return nullptr;
}

//-----

static auto _Text_to_bytes(const fd::string_view text_src)
{
    unknown_bytes_range bytes;
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

    return bytes;
}

void* signature_finder::operator()(const fd::string_view sig, const bool raw) const
{
    const void* ret;

    if (raw)
        ret = _Find_block(from, to, (pointer)sig.data(), sig.size());
    else
        ret = _Find_unk_block(from, to, _Text_to_bytes(sig));
    return const_cast<void*>(ret);
}

void* signature_finder::operator()(const pointer begin, const size_t mem_size) const
{
    return const_cast<void*>(_Find_block(from, to, begin, mem_size));
}

module;

#include <fd/assert.h>

#include <algorithm>
#include <memory>
#include <ranges>
#include <span>
#include <vector>

module fd.signature;

static bool _Validate_signature(const fd::string_view rng)
{
    if (rng.starts_with('?'))
        return false;
    if (rng.starts_with(' '))
        return false;
    if (rng.ends_with(' '))
        return false;

    uint8_t qmarks  = 0;
    uint8_t spaces  = 0;
    uint8_t counter = 0;
    for (const auto c : rng)
    {
        switch (c)
        {
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        case 'a':
        case 'b':
        case 'c':
        case 'd':
        case 'e':
        case 'f':
        case 'A':
        case 'B':
        case 'C':
        case 'D':
        case 'E':
        case 'F': {
            if (qmarks > 0)
                return false;
            ++counter;
            if (counter > 2)
                return false;
            qmarks = spaces = 0;
            break;
        }
        case '?': {
            ++qmarks;
            if (qmarks > 2)
                return false;
            counter = spaces = 0;
            break;
        }
        case ' ': {
            ++spaces;
            if (spaces > 1)
                return false;
            counter = qmarks = 0;
            break;
        }
        default: {
            return false;
        }
        }
    }

    return true;
}

static pointer _Find_block_memchr(const size_t rng_size, const size_t limit, const pointer start0, const pointer start2)
{
    size_t offset = 0;
    do
    {
        const auto start1 = static_cast<pointer>(std::memchr(start0 + offset, start2[0], limit - offset));
        if (!start1)
            break;

        if (std::memcmp(start1, start2, rng_size) == 0)
            return start1;

        offset = std::distance(start0, start1) + 1;
    }
    while (offset <= limit);

    return nullptr;
}

// 10-100x slower than memchr version (debug)
static pointer _Find_block_memcmp(const size_t rng_size, const size_t limit, const pointer start0, const pointer start2)
{
    for (size_t offset = 0; offset < limit; ++offset)
    {
        const auto start1 = start0 + offset;
        if (std::memcmp(start1, start2, rng_size) == 0)
            return start1;
    }

    return nullptr;
}

//-----

using mem_block = std::span<const uint8_t>;

static mem_block _Find_block_handmade(const mem_block from, const mem_block other)
{
    const auto rng_size = other.size();
    const auto limit    = from.size() - rng_size;

    const auto start0 = from.data();
    const auto start2 = other.data();

    if (rng_size == 1)
    {
        const auto found = std::memchr(start0, *start2, limit);
        if (found)
            return { static_cast<pointer>(found), 1 };
    }
    else
    {
        const auto ptr = _Find_block_memchr(rng_size, limit, start0, start2);
        if (ptr)
            return { ptr, rng_size };
    }

    return {};
}

// 2x slower than iters version (debug)
static mem_block _Find_block_ranges(const mem_block from, const mem_block other)
{
    const auto found = std::ranges::search(from, other);
    if (!found.empty())
        return { found.data(), found.size() };
    return {};
}

// 3x slower than handmade verison (debug)
static mem_block _Find_block_iters(const mem_block from, const mem_block other)
{
    const auto found = std::search(from.begin(), from.end(), other.begin(), other.end());
    if (found != from.end())
        return { &*found, other.size() };
    return {};
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
            ++bytes.back().skip;
        }
        else
        {
            // write previous part
            if (bytes.back().skip > 0)
                bytes.emplace_back();

            switch (b.size())
            {
            case 1:
                bytes.back().part.push_back(to_num(b[0]));
                break;
            case 2:
                bytes.back().part.push_back(to_num(b[0]) * 16 + to_num(b[1]));
                break;
            default:
                FD_ASSERT_UNREACHABLE("Incorrect string validation!");
            }
        }
    }

    return bytes;
}

//~5x slower than modern version (debug)
static mem_block _Find_unk_block(const mem_block from, const unknown_bytes_range& unkbytes)
{
    const auto _last_pos = from.data() + from.size() - unkbytes.bytes_count();
    for (auto _pos = from.data(); _pos <= _last_pos;)
    {
        auto inner_pos = _pos;
        for (const auto& [part, skip] : unkbytes)
        {
            for (const auto chr : part)
            {
                if (chr != *inner_pos++)
                    goto _NO_RETURN;
            }
            inner_pos += skip;
        }

        return { _pos, inner_pos };

    _NO_RETURN:
        if (inner_pos == _pos)
            ++_pos;
        else
            _pos = inner_pos;
    }

    return {};
}

static mem_block _Find_unk_block_modern(const mem_block from, const unknown_bytes_range& unkbytes)
{
    const auto unkbytes_count = unkbytes.bytes_count();
    if (from.size() < unkbytes_count)
        return {};

    const auto unkbytes_0 = unkbytes.begin();
    const mem_block unkbytes_first_block(unkbytes_0->part.data(), unkbytes_0->part.size());
    const auto unkbytes_first_skip = unkbytes_0->skip;

    if (unkbytes.size() == 1)
    {
        const auto found = _Find_block_handmade(from, unkbytes_first_block);
        if (!found.empty() && unkbytes_first_skip > 0)
        {
            /* const auto mem_after = from.shift_to(found.data() + found.size());
            if (mem_after.size() < unkbytes_first_skip)
                return {}; */
            const auto mem_after = std::distance(found.data() + found.size(), from.data() + from.size());
            if (mem_after < unkbytes_first_skip)
                return {};
        }
        return found;
    }
    else
    {
        const std::span unkbytes_except_first(unkbytes_0 + 1, unkbytes.end());

        auto current_pos             = from.data();
        const auto last_pos          = current_pos + from.size();
        const auto last_readable_pos = last_pos - unkbytes_count;

        do
        {
            const mem_block found0 = _Find_block_handmade({ current_pos, last_pos }, unkbytes_first_block);
            if (found0.empty())
                break;

            current_pos  = found0.data() + found0.size();
            auto tmp_pos = current_pos + unkbytes_first_skip;

            bool found = true;
            for (const auto& [part, skip] : unkbytes_except_first)
            {
                const auto buff_size = part.size();
                if (std::memcmp(tmp_pos, part.data(), buff_size) != 0)
                {
                    found = false;
                    break;
                }

                current_pos = tmp_pos + buff_size;
                tmp_pos     = current_pos + skip;
            }

            if (found)
                return { found0.data(), unkbytes_count };
        }
        while (current_pos <= last_readable_pos);

        return {};
    }
}

//-----

pointer signature_finder::operator()(const fd::string_view sig, const bool raw) const
{
    if (raw)
        return _Find_block_handmade({ from, to }, { (pointer)sig.data(), sig.size() }).data();

    FD_ASSERT(_Validate_signature(sig));
    const auto bytes = _Text_to_bytes(sig);

    //-----------------

    return _Find_unk_block_modern({ from, to }, bytes).data();
}

pointer signature_finder::operator()(pointer begin, const size_t mem_size) const
{
    return _Find_block_handmade({ from, to }, { begin, mem_size }).data();
}

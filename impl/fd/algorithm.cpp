#include <fd/algorithm.h>
#include <fd/system.h>

#include <intrin.h>

#include <algorithm>
#include <chrono>

namespace fd
{
#if 1
#define WRAP_PREPARE(_NAME_) \
    template <typename T>    \
    static constexpr void* _NAME_ = nullptr;

#define WRAP_HELPER(_NAME_, _TYPE_, _FN_)                     \
    template <>                                               \
    static constexpr auto _NAME_<_TYPE_> = [](auto... args) { \
        return _FN_(args...);                                 \
    };
#else
#define WRAP_PREPARE(_NAME_)
#define WRAP_HELPER(_NAME_, _TYPE_, _FN_)               \
    template <std::same_as<_TYPE_> T, typename... Args> \
    static auto _NAME_(Args... args)                    \
    {                                                   \
        return _FN_(args...);                           \
    }
#endif

#define WRAP(_NAME_, _FN1_, _FN2_, _FN4_, _FN8_) \
    WRAP_PREPARE(_NAME_);                        \
    WRAP_HELPER(_NAME_, uint8_t, _FN1_);         \
    WRAP_HELPER(_NAME_, uint16_t, _FN2_);        \
    WRAP_HELPER(_NAME_, uint32_t, _FN4_);        \
    WRAP_HELPER(_NAME_, uint64_t, _FN8_);

WRAP(_SetAVX512, _mm512_set1_epi8, _mm512_set1_epi16, _mm512_set1_epi32, _mm512_set1_epi64);
WRAP(_CmpAVX512, _mm512_cmpeq_epi8_mask, _mm512_cmpeq_epi16_mask, _mm512_cmpeq_epi32_mask, _mm512_cmpeq_epi64_mask);
WRAP(_SetAVX2, _mm256_set1_epi8, _mm256_set1_epi16, _mm256_set1_epi32, _mm256_set1_epi64x);
WRAP(_CmpAVX2, _mm256_cmpeq_epi8, _mm256_cmpeq_epi16, _mm256_cmpeq_epi32, _mm256_cmpeq_epi64);
WRAP(_SetSSE2, _mm_set1_epi8, _mm_set1_epi16, _mm_set1_epi32, _mm_set1_epi64x);
WRAP(_CmpSSE2, _mm_cmpeq_epi8, _mm_cmpeq_epi16, _mm_cmpeq_epi32, _mm_cmpeq_epi64);

static size_t _bytes_count(const void* left, const void* right)
{
    return std::distance(static_cast<const uint8_t*>(left), static_cast<const uint8_t*>(right));
}

static void* _add_bytes(const void* ptr, const size_t count)
{
    union
    {
        const void*    hint;
        const uint8_t* target;

        void* result;
    } helper;

    helper.hint = ptr;
    helper.target += count;
    return helper.result;
}

template <typename T>
static void* _find_memchr(void* rngStart, void* rngEnd, const T value, size_t bytesCount = 0)
{
    if (bytesCount == 0)
        bytesCount = _bytes_count(rngStart, rngEnd);

    const int val8 = reinterpret_cast<const uint8_t&>(value);

    if constexpr (sizeof(T) > 1)
    {
        for (void* result; bytesCount < sizeof(T); bytesCount = _bytes_count(result, rngEnd))
        {
            result = memchr(rngStart, val8, bytesCount);
            if (!result)
                break;
            if (*static_cast<T*>(result) == value)
                return result;
        }
    }
    else
    {
        const auto result = memchr(rngStart, val8, bytesCount);
        if (result)
            return result;
    }

    return rngEnd;
}

template <typename T, bool Aligned>
class simd_iterator
{
    T* pos_;

  public:
    simd_iterator(T* pos)
        : pos_(pos)
    {
    }

    simd_iterator& operator++()
    {
        ++pos_;
        return *this;
    }

    T    operator*() const;
    bool operator==(const simd_iterator&) const = default;

    operator T*() const
    {
        return pos_;
    }

    simd_iterator operator+(size_t offset) const
    {
        return pos_ + offset;
    }
};

template <typename T, bool Aligned>
// ReSharper disable once CppInconsistentNaming
static size_t distance(const simd_iterator<T, Aligned>& left, const simd_iterator<T, Aligned>& right)
{
    return std::distance<T*>(left, right);
}

template <>
__m512i simd_iterator<__m512i, false>::operator*() const
{
    return _mm512_loadu_si512(pos_);
}

template <>
__m512i simd_iterator<__m512i, true>::operator*() const
{
    return _mm512_load_si512(pos_);
}

template <>
__m256i simd_iterator<__m256i, false>::operator*() const
{
    return _mm256_loadu_si256(pos_);
}

template <>
__m256i simd_iterator<__m256i, true>::operator*() const
{
    return _mm256_load_si256(pos_);
}

template <>
__m128i simd_iterator<__m128i, true>::operator*() const
{
    return _mm_load_si128(pos_);
}

template <>
__m128i simd_iterator<__m128i, false>::operator*() const
{
    return _mm_loadu_si128(pos_);
}

template <typename T>
static void* _find_avx2(void* rngStart, void* rngEnd, const T value, size_t bytesCount = 0)
{
    if (bytesCount == 0)
        bytesCount = _bytes_count(rngStart, rngEnd);

    if (bytesCount < sizeof(__m256i))
        return nullptr;

    const auto value256 = _SetAVX2<T>(value);

    using itr_t = simd_iterator<__m256i, true>;

    itr_t      start = static_cast<__m256i*>(rngStart);
    const auto end   = start + bytesCount / sizeof(__m256i);

    for (; start != end; ++start)
    {
        const auto mask = _mm256_movemask_epi8(_CmpAVX2<T>(value256, *start));
        if (mask != 0)
        {
            const auto offset = _tzcnt_u32(mask);
            return _add_bytes(start, offset);
        }
    }

    return rngEnd;
}

template <typename T>
static void* _find_sse2(void* rngStart, void* rngEnd, const T value, size_t bytesCount = 0)
{
    if (bytesCount == 0)
        bytesCount = _bytes_count(rngStart, rngEnd);

    if (bytesCount < sizeof(__m128i))
        return nullptr;

    const auto value128 = _SetSSE2<T>(value);

    using itr_t = simd_iterator<__m128i, true>;

    itr_t      start = static_cast<__m128i*>(rngStart);
    const auto end   = start + bytesCount / sizeof(__m128i);

    for (; start != end; ++start)
    {
        const auto mask = _mm_movemask_epi8(_CmpSSE2<T>(value128, *start));
        if (mask != 0)
        {
            unsigned long offset;
            _BitScanForward(&offset, mask);
            return _add_bytes(start, offset);
        }
    }

    return rngEnd;
}

template <typename T>
static void* _find_select(void* rngStart, void* rngEnd, const T value)
{
    const auto bytesCount = _bytes_count(rngStart, rngEnd);
    if (system::CPU.AVX2)
    {
        auto result = _find_avx2(rngStart, rngEnd, value, bytesCount);
        if (result)
            return result;
    }
    if (system::CPU.SSE2)
    {
        auto result = _find_sse2(rngStart, rngEnd, value, bytesCount);
        if (result)
            return result;
    }

    return _find_memchr(rngStart, rngEnd, value, bytesCount);
}

// ReSharper disable once CppInconsistentNaming
void* _find_1(const void* rngStart, const void* rngEnd, const uint8_t val)
{
    return _find_select(const_cast<void*>(rngStart), const_cast<void*>(rngEnd), val);
}

// ReSharper disable once CppInconsistentNaming
void* _find_2(const void* rngStart, const void* rngEnd, const uint16_t val)
{
    return _find_select(const_cast<void*>(rngStart), const_cast<void*>(rngEnd), val);
}

// ReSharper disable once CppInconsistentNaming
void* _find_4(const void* rngStart, const void* rngEnd, const uint32_t val)
{
    return _find_select(const_cast<void*>(rngStart), const_cast<void*>(rngEnd), val);
}

// ReSharper disable once CppInconsistentNaming
void* _find_8(const void* rngStart, const void* rngEnd, const uint64_t val)
{
    return _find_select(const_cast<void*>(rngStart), const_cast<void*>(rngEnd), val);
}

size_t test_algorithms()
{
    using clock = std::chrono::high_resolution_clock;

    auto run_test = [](auto fn) {
        auto start = clock::now();
        auto val   = fn();
        auto stop  = clock::now();
        return std::pair(val, stop - start);
    };

    const auto set = [](auto& arr, size_t pos, auto... args) {
        ((arr[pos++] = args), ...);
    };

    constexpr auto arrS = 1024 * 1024;

    auto intA       = new int[arrS];
    int  intTestA[] = { 100, 1, 2, 3 };
    set(intA, arrS - 10, 100, 1, 2, 3);

    auto charA       = new char[arrS];
    char charTestA[] = { 100, 1, 2, 3 };
    set(charA, arrS - 10, 100, 1, 2, 3);

    std::vector<void*> dummy;
    auto               a = clock::now();
    dummy.push_back(&a);

#define RUN_TEST(_NAME_, ...)                              \
    [[maybe_unused]] volatile auto _NAME_ = run_test([&] { \
        return __VA_ARGS__;                                \
    });                                                    \
    dummy.insert(dummy.end(), (void*)&_NAME_);

    RUN_TEST(stdFindInt, std::find(intA, intA + arrS, 2));
    RUN_TEST(stdFindChar, std::find(charA, charA + arrS, (char)2));

    RUN_TEST(stdSearchInt, std::search(intA, intA + arrS, std::begin(intTestA), std::end(intTestA)));
    RUN_TEST(stdSearchChar, std::search(charA, charA + arrS, std::begin(charTestA), std::end(charTestA)));

    RUN_TEST(fdFindInt, fd::find(intA, intA + arrS, 2));
    RUN_TEST(fdFindChar, fd::find(charA, charA + arrS, (char)2));
    RUN_TEST(fdFindChar2, fd::find(charA, charA + arrS, 2));

    RUN_TEST(fdSearchInt, fd::find(intA, intA + arrS, std::begin(intTestA), std::end(intTestA)));
    RUN_TEST(fdSearchChar, fd::find(charA, charA + arrS, std::begin(charTestA), std::end(charTestA)));

    return dummy.size();
}

}
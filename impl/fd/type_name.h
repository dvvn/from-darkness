#pragma once

#include <fd/algorithm.h>
#include <fd/chars_cache.h>

namespace fd
{
template <typename T>
static constexpr decltype(auto) _raw_type_name()
{
    return __FUNCSIG__;
}

template <template <typename...> class T>
static constexpr decltype(auto) _raw_type_name()
{
    return __FUNCSIG__;
}

template <size_t S>
static constexpr string_view _clamp_raw_type_name(const char (&rawName)[S])
{
    auto charsCount = S;
    if (*std::rbegin(rawName) == '\0')
        --charsCount;
    const string_view name(rawName, charsCount);
    return { rawName + name.find('<') + 1, rawName + name.rfind('>') };
}

template <size_t BuffSize>
struct clamped_type_name
{
    char   buff[BuffSize];
    size_t strSize;
    size_t nativeOffset;

    constexpr clamped_type_name(const string_view rawName)
        : buff()
        , strSize(0)
        , nativeOffset(string_view::npos)
    {
        char tmpBuff[BuffSize];
        copy(rawName, tmpBuff);
        fill(std::rbegin(tmpBuff), rawName.size() - BuffSize, '\0');

        // remove bad spaces
        auto        skipNext = false;
        string_view strBefore;
        for (auto& c : tmpBuff)
        {
            switch (c)
            {
            case ',': /*template separator*/ {
                skipNext = true;
                break;
            }
            case 't': /*const*/ {
                if (strBefore.ends_with("cons"))
                    skipNext = true;
                break;
            }
            case 'c': /*static*/ {
                if (strBefore.ends_with("stati"))
                    skipNext = true;
                break;
            }
            case ' ': {
                strBefore = {};
                if (skipNext)
                    continue;
                c = '\0';
            }
            default: {
                skipNext = false;
                break;
            }
            }
            if (strBefore.empty())
                strBefore = { &c, 1 };
            else
                strBefore = { strBefore.data(), strBefore.size() + 1 };
        }

        // remove bad words
        constexpr string_view badWords[] = { "struct", "class", "enum", "union" };
        for (const auto word : badWords)
        {
            for (const string_view str(tmpBuff, BuffSize);;)
            {
                const auto pos = str.find(word);
                if (pos == string_view::npos)
                    break;
                fill(tmpBuff + pos, word.size(), '\0');
            }
        }

        // write buffer
        for (auto c : tmpBuff)
        {
            if (c == '\0')
                continue;
            buff[strSize++] = c;
        }

        nativeOffset = rawName.find(buff, 0, strSize); // NOLINT(cppcoreguidelines-prefer-member-initializer)
    }

    constexpr bool native() const
    {
        return nativeOffset != string_view::npos;
    }

    template <size_t StrSize = BuffSize>
    constexpr chars_cache<false, char, StrSize> clone() const
    {
        return buff;
    }

    constexpr string_view view(const string_view rawName) const
    {
        return rawName.substr(nativeOffset, strSize);
    }

    constexpr string_view view_before(const string_view rawName, const char chr) const
    {
        const auto nativeStr = rawName.substr(nativeOffset, strSize);
        return nativeStr.substr(0, nativeStr.find(chr));
    }

    constexpr size_t offset_to(const char chr) const
    {
        return std::distance(buff, _find(buff, strSize, chr));
    }
};

template <typename T>
static constexpr bool _IsClassOrUnion = std::is_class_v<T> || std::is_union_v<T>;

/*template <typename T>
struct remove_all_pointers : std::conditional_t<std::is_pointer_v<T>, remove_all_pointers<std::remove_pointer_t<T>>, std::type_identity<T>>
{
};*/

template <typename T, bool V = std::is_pointer_v<T>>
struct remove_all_pointers;

template <typename T>
struct remove_all_pointers<T, false>
{
    using type = T;
};

template <typename T>
struct remove_all_pointers<T, true> : remove_all_pointers<std::remove_pointer_t<T>>
{
};

template <typename T>
using remove_all_pointers_t = typename remove_all_pointers<T>::type;

template <typename T>
using raw_type_t = remove_all_pointers_t<std::decay_t<T>>;

template <typename T>
struct type_name_precached;

#define FD_TYPE_NAME_PRECACHE(_TYPE_)                \
    template <>                                      \
    struct type_name_precached<_TYPE_>               \
    {                                                \
        static constexpr string_view name = #_TYPE_; \
    };

FD_TYPE_NAME_PRECACHE(char);
FD_TYPE_NAME_PRECACHE(wchar_t);
FD_TYPE_NAME_PRECACHE(char8_t);
FD_TYPE_NAME_PRECACHE(char16_t);
FD_TYPE_NAME_PRECACHE(char32_t);

FD_TYPE_NAME_PRECACHE(int8_t);
FD_TYPE_NAME_PRECACHE(uint8_t);
FD_TYPE_NAME_PRECACHE(int16_t);
FD_TYPE_NAME_PRECACHE(uint16_t);
FD_TYPE_NAME_PRECACHE(int32_t);
FD_TYPE_NAME_PRECACHE(uint32_t);
FD_TYPE_NAME_PRECACHE(int64_t);
FD_TYPE_NAME_PRECACHE(uint64_t);

FD_TYPE_NAME_PRECACHE(float);
FD_TYPE_NAME_PRECACHE(double);
FD_TYPE_NAME_PRECACHE(long double);

FD_TYPE_NAME_PRECACHE(void*);
FD_TYPE_NAME_PRECACHE(char*);
FD_TYPE_NAME_PRECACHE(const char*);
FD_TYPE_NAME_PRECACHE(long);
FD_TYPE_NAME_PRECACHE(bool);

template <typename T>
concept have_cached_type_name = requires { type_name_precached<T>::name; };

template <typename T>
constexpr auto type_name()
{
    if constexpr (have_cached_type_name<T>)
    {
        return type_name_precached<T>::name;
    }
    else
    {
        constexpr auto rawName = _clamp_raw_type_name(_raw_type_name<T>());
        using raw_t            = raw_type_t<T>;
        if constexpr (!_IsClassOrUnion<raw_t> && !std::is_enum_v<raw_t>)
        {
            return rawName;
        }
        else
        {
            constexpr clamped_type_name<rawName.size()> clampedName(rawName);
            if constexpr (clampedName.native())
                return clampedName.view(rawName);
            else
                return clampedName.template clone<clampedName.strSize>();
        }
    }
}

template <template <typename...> class T>
constexpr auto type_name()
{
    constexpr auto                              rawName = _clamp_raw_type_name(_raw_type_name<T>());
    constexpr clamped_type_name<rawName.size()> clampedName(rawName);
    if constexpr (clampedName.native())
        return clampedName.view(rawName);
    else
        return clampedName.template clone<clampedName.strSize>();
};

// for std::array
template <template <typename, size_t> class T>
constexpr auto type_name()
{
    constexpr auto                              rawName = _clamp_raw_type_name(_raw_type_name<T<int, 1>>());
    constexpr clamped_type_name<rawName.size()> clampedName(rawName);
    if constexpr (clampedName.native())
        return clampedName.view_before(rawName, '<');
    else
        return clampedName.template clone<clampedName.offset_to('<')>();
}

#ifdef _DEBUG_OFF
static_assert(type_name<float>() == "float");
static_assert(type_name<int32_t>() == type_name<int>());
static_assert(type_name<std::char_traits>() == "std::char_traits");
static_assert(type_name<std::char_traits<char>>() == "std::char_traits<char>");
static_assert(type_name<std::array>() == "std::array");
static_assert(type_name<std::array<int, 1>>() == "std::array<int,1>");
static_assert(type_name<std::exception>() == "std::exception");
static_assert(type_name<const std::exception>() == "const std::exception");
#endif

//------------------

struct template_info
{
    const char* name;
    bool        partial;
};

static constexpr auto _find_first_char(const char* ptr, const char c)
{
    return std::char_traits<char>::find(ptr, static_cast<size_t>(-1), c);
};

static constexpr auto _find_last_char(const char* ptr, const char c)
{
    const char* out = nullptr;
    for (; *ptr != '\0'; ++ptr)
    {
        if (*ptr == c)
            out = ptr;
    }
    return out;
};

static constexpr size_t _get_offset_for(auto fn, const char* ptr, const char c)
{
    const auto found = fn(ptr, c);
    return std::distance(ptr, found);
};

static constexpr bool _same_template(const template_info infoL, const template_info infoR)
{
    // every __FUNCSIG__ have own pointer, if they are same - strings are same
    if (infoL.name == infoR.name)
        return true;
    // we know the strings are different, tempalte part can't be found
    if (infoL.partial == infoR.partial)
        return false;

    const auto [strPartial, strFull] = [&] {
        // skip XXXXtype_name_raw
        const auto offset = _get_offset_for(_find_first_char, infoL.name, '<') + 1;
        const auto strL   = infoL.name + offset;
        const auto strR   = infoR.name + offset;
        return infoL.partial ? std::pair(strL, strR) : std::pair(strR, strL);
    }();

    // offset_to end of partial tempalte
    const auto limit = _get_offset_for(_find_last_char, strPartial, '>');
    if (!_ptr_equal(strPartial, strFull, limit))
        return false;
    // check are full template same as given part
    return strFull[limit] == '<';
}

template <class T1, template <typename...> class T2>
constexpr bool same_template()
{
    using raw_t1 = raw_type_t<T1>;
    static_assert(_IsClassOrUnion<raw_t1>, "same_template works only with classes!");
    return _same_template({ _raw_type_name<raw_t1>(), false }, { _raw_type_name<T2>(), true });
}

template <template <typename...> class T1, class T2>
constexpr bool same_template()
{
    return same_template<T2, T1>();
}

template <template <typename...> class T1, template <typename...> class T2>
constexpr bool same_template()
{
    return _same_template({ _raw_type_name<T1>(), true }, { _raw_type_name<T2>(), true });
}

template <class T1, class T2>
constexpr bool same_template()
{
    using raw_t1 = raw_type_t<T1>;
    using raw_t2 = raw_type_t<T2>;
    static_assert(_IsClassOrUnion<raw_t1> && _IsClassOrUnion<raw_t2>, "same_template works only with classes!");
    return _same_template({ _raw_type_name<raw_t1>(), false }, { _raw_type_name<raw_t2>(), false });
}

#ifdef _DEBUG
// static_assert(same_template<std::array<int, 2>, std::array>());
// static_assert(!same_template<std::array<int, 2>, std::array<int, 3>>());
static_assert(same_template<std::char_traits, std::char_traits<char>>());
static_assert(!same_template<std::char_traits<int>, std::char_traits<char>>());
#endif
} // namespace fd
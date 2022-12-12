#pragma once

#include <fd/chars_cache.h>
#include <fd/functional.h>

#include <algorithm>
#include <ranges>

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

    static constexpr string_view _clamp_raw_type_name(const string_view rawName)
    {
        return { rawName.data() + rawName.find('<') + 1, rawName.data() + rawName.rfind('>') };
    }

    template <size_t BuffSize>
    struct clamped_type_name
    {
        char buff[BuffSize];
        size_t strSize;
        size_t nativeOffset;

        constexpr clamped_type_name(const string_view rawName)
            : buff()
        {
            constexpr auto removeBadSpaces = [](bool& skipNext, string_view& strBefore, char& c) {
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
                case ' ': {
                    strBefore = {};
                    if (skipNext)
                        return;
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
            };
            constexpr auto removeBadWords = [](char* buff, const string_view word) {
                for (const string_view str(buff, BuffSize);;)
                {
                    const auto pos = str.find(word);
                    if (pos == string_view::npos)
                        break;
                    std::ranges::fill_n(buff + pos, word.size(), '\0');
                }
            };
            constexpr auto skipNull = [](const char c) {
                return c != '\0';
            };
            constexpr std::array badWords = { "struct", "class", "enum", "union" };

            char tmpBuff[BuffSize];
            std::ranges::copy(rawName, tmpBuff);
            std::ranges::for_each(tmpBuff, bind_front(removeBadSpaces, false, string_view()));
            std::ranges::for_each(badWords, bind_front(removeBadWords, tmpBuff));

            const auto buffEnd = std::ranges::copy(tmpBuff | std::views::filter(skipNull), buff).out;
            strSize            = std::distance(buff, buffEnd);
            nativeOffset       = rawName.find(buff, 0u, strSize);
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

        constexpr size_t find(const char chr) const
        {
            return string_view(buff, strSize).find(chr);
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
    constexpr auto type_name()
    {
        constexpr auto rawName = _clamp_raw_type_name(_raw_type_name<T>());
        using raw_t            = raw_type_t<T>;
        if constexpr (_IsClassOrUnion<raw_t> || std::is_enum_v<raw_t>)
        {
            constexpr clamped_type_name<rawName.size()> clampedName(rawName);
            if constexpr (clampedName.native())
                return clampedName.view(rawName);
            else
                return clampedName.template clone<clampedName.strSize>();
        }
        else
        {
            return rawName;
        }
    }

    template <template <typename...> class T>
    constexpr auto type_name()
    {
        constexpr auto rawName = _clamp_raw_type_name(_raw_type_name<T>());
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
        constexpr auto rawName = _clamp_raw_type_name(_raw_type_name<T<int, 1>>());
        constexpr clamped_type_name<rawName.size()> clampedName(rawName);
        if constexpr (clampedName.native())
            return clampedName.view_before(rawName, '<');
        else
            return clampedName.template clone<clampedName.find('<')>();
    }

    static_assert(type_name<int>() == "int");
    static_assert(type_name<int32_t>() == type_name<int>());
    static_assert(type_name<std::char_traits>() == "std::char_traits");
    static_assert(type_name<std::char_traits<char>>() == "std::char_traits<char>");
    static_assert(type_name<std::array>() == "std::array");
    static_assert(type_name<std::array<int, 1>>() == "std::array<int,1>");
    static_assert(type_name<std::exception>() == "std::exception");
    static_assert(type_name<const std::exception>() == "const std::exception");

    //------------------

    struct template_info
    {
        const char* name;
        bool partial;
    };

    static constexpr bool _same_template(const template_info infoL, const template_info infoR)
    {
        // every __FUNCSIG__ have own pointer, if they are same - strings are same
        if (infoL.name == infoR.name)
            return true;
        // we know the strings are different, tempalte part can't be found
        if (infoL.partial == infoR.partial)
            return false;

        using char_traits = std::char_traits<char>;

        constexpr auto findFirstChar = [](const char* ptr, const char c) {
            return char_traits::find(ptr, static_cast<size_t>(-1), c);
        };
        constexpr auto findLastChar = [](const char* ptr, const char c) {
            const char* out = nullptr;
            for (; *ptr != '\0'; ++ptr)
            {
                if (*ptr == c)
                    out = ptr;
            }
            return out;
        };
        constexpr auto getOffset = [](auto fn, const char* ptr, const char c) -> size_t {
            const auto found = invoke(fn, ptr, c);
            // ReSharper disable once CppUnreachableCode
            return std::distance(ptr, found);
        };
        // skip XXXXtype_name_raw
        const auto offset = invoke(getOffset, findFirstChar, infoL.name, '<') + 1;

        const auto strL = infoL.name + offset;
        const auto strR = infoR.name + offset;

        const char *strPartial, *strFull;
        if (infoL.partial)
        {
            strPartial = strL;
            strFull    = strR;
        }
        else
        {
            strPartial = strR;
            strFull    = strL;
        }
        // find end of partial tempalte
        const auto limit = invoke(getOffset, findLastChar, strPartial, '>');
        if (char_traits::compare(strPartial, strFull, limit) != 0)
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

    // static_assert(same_template<std::array<int, 2>, std::array>());
    // static_assert(!same_template<std::array<int, 2>, std::array<int, 3>>());
    static_assert(same_template<std::char_traits, std::char_traits<char>>());
    static_assert(!same_template<std::char_traits<int>, std::char_traits<char>>());
} // namespace fd
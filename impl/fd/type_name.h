#pragma once

#include <fd/string.h>

#include <algorithm>

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

    struct type_name_builder : string
    {
        constexpr size_t mark_zeros(const string_view substr)
        {
            const auto substrSize = substr.size();
            size_t found          = 0;
            size_t pos            = 0;
            for (;;)
            {
                pos = this->find(substr, pos);
                if (pos == this->npos)
                    break;
                ++found;
                std::fill_n(this->data() + pos, substrSize, '\0');
                pos += substrSize;
            }
            return found * substrSize;
        }

        constexpr size_t mark_zeros(const char chr)
        {
            size_t found = 0;
            for (auto& c : *this)
            {
                if (c != chr)
                    continue;
                ++found;
                c = '\0';
            }
            return found;
        }

        constexpr size_t remove_bad_spaces()
        {
            size_t found  = 0;
            auto skipNext = false;
            for (auto& c : *this)
            {
                switch (c)
                {
                case ',': {
                    skipNext = true;
                    break;
                }
                case ' ': {
                    if (skipNext)
                        break;
                    c = '\0';
                    ++found;
                }
                default: {
                    skipNext = false;
                    break;
                }
                }
            }

            return found;
        }

        constexpr bool erase_zeros(const bool notSure)
        {
            string temp;
            if (!notSure)
                temp.reserve(this->size());
            for (const auto chr : *this)
            {
                if (chr == '\0')
                    continue;
                temp += chr;
            }
            if (temp.empty())
            {
                if (!notSure)
                    (void)temp.at(temp.size()); // throw a error
                return false;
            }
            this->assign(std::move(temp));
            return true;
        }

        //------

        template <typename... Args>
        constexpr bool smart_erase(const Args... args)
        {
            const auto zeros = (this->mark_zeros(args) + ...) + this->remove_bad_spaces();
            return zeros > 0 && this->erase_zeros(false);
        }
    };

    template <size_t Size>
    class type_name_getter
    {
        char buffer_[Size];
        size_t size_;
        bool native_;

      public:
        constexpr type_name_getter(const string_view clampedName, const bool isObject)
            : buffer_()
        {
            if (!isObject)
            {
                std::ranges::copy(clampedName, buffer_);
                size_   = clampedName.size();
                native_ = true;
            }
            else
            {
                type_name_builder name;
                name.assign(clampedName);
                native_ = !name.smart_erase("struct", "class", "enum", "union");
                std::ranges::copy(name, buffer_);
                size_ = name.size();
            }
        }

        constexpr const char* data() const
        {
            return buffer_;
        }

        constexpr size_t size() const
        {
            return size_;
        }

        constexpr bool native() const
        {
            return native_;
        }

        constexpr size_t find(const char chr) const
        {
            return std::distance<const char*>(buffer_, std::char_traits<char>::find(buffer_, size_, chr));
        }
    };

    template <typename T>
    static constexpr bool _IsClassOrUnion = std::is_class_v<T> || std::is_union_v<T>;

    template <typename T>
    constexpr auto type_name()
    {
        constexpr auto name = _clamp_raw_type_name(_raw_type_name<T>());
        if constexpr (_IsClassOrUnion<T> || std::is_enum_v<T>)
        {
            constexpr type_name_getter<name.size()> getter(name, true);
            if constexpr (getter.native())
                return name;
            else
                return chars_cache<false, char, getter.size()>(getter.data());
        }
        else
        {
            return name;
        }
    }

    template <template <typename...> class T>
    constexpr auto type_name()
    {
        constexpr auto name = _clamp_raw_type_name(_raw_type_name<T>());
        constexpr type_name_getter<name.size()> getter(name, true);
        if constexpr (getter.native())
            return name;
        else
            return chars_cache<false, char, getter.size()>(getter.data());
    };

    // for std::array
    template <template <typename, size_t> class T>
    constexpr auto type_name()
    {
        constexpr auto name = _clamp_raw_type_name(_raw_type_name<T<int, 1>>());
        constexpr type_name_getter<name.size()> getter(name, true);
        if constexpr (getter.native())
            return string_view(name.data(), name.find('<'));
        else
            return chars_cache<false, char, getter.find('<')>(getter.data());
    }

    static_assert(type_name<int>() == "int");
    static_assert(type_name<int32_t>() == type_name<int>());
    static_assert(type_name<std::char_traits>() == "std::char_traits");
    static_assert(type_name<std::char_traits<char>>() == "std::char_traits<char>");
    static_assert(type_name<std::array>() == "std::array");
    static_assert(type_name<std::array<int, 1>>() == "std::array<int,1>");
    static_assert(type_name<std::exception>() == "std::exception");

    //------------------

    struct template_info
    {
        const char* name;
        bool partial;
    };

    static constexpr bool _same_template(const template_info infoL, const template_info infoR)
    {
#if 1
        // every __FUNCSIG__ have own pointer, if they are same - strings are same
        if (infoL.name == infoR.name)
            return true;
        // we know the strings are different, tempalte part can't be found
        if (infoL.partial == infoR.partial)
            return false;

        using char_traits = std::char_traits<char>;

        constexpr auto find_existing_char = [](const char* ptr, const char c) {
            return char_traits::find(ptr, static_cast<size_t>(-1), c);
        };
        constexpr auto find_last_existing_char = [](const char* ptr, const char c) -> const char* {
            const auto end = ptr + char_traits::length(ptr);
            for (auto pos = end - 1; pos >= ptr; --pos)
            {
                if (*pos == c)
                    return pos;
            }
            return nullptr;
        };

        // skip XXXXtype_name_raw
        const auto offset = std::distance(infoL.name, find_existing_char(infoL.name, '<')) + 1;

        const auto l_str = infoL.name + offset;
        const auto r_str = infoR.name + offset;

        const char *partial_str, *full_str;
        if (infoL.partial)
        {
            partial_str = l_str;
            full_str    = r_str;
        }
        else
        {
            partial_str = r_str;
            full_str    = l_str;
        }
        // find end of partial tempalte
        const auto limit = std::distance(partial_str, find_last_existing_char(partial_str, '>'));
        if (char_traits::compare(l_str, r_str, limit) != 0)
            return false;
        // check are full template same as given part
        return full_str[limit] == '<';

#else
        if (left == right)
            return true;
        // skip XXXXtype_name_raw
        do
            ++left;
        while (*right++ != '<');

        for (;;)
        {
            auto l = *left++;
            auto r = *right++;

            if (l != r)
            {
                return l == '>' || r == '>'     // partial template _Class
                       || l == '<' || r == '<'; // full template _Class<XXX>;
            }
            if (l == '\0')
                return false;
            if (l == '<' || l == '>')
                return true;
        }
#endif
    }

    template <class T1, template <typename...> class T2>
    constexpr bool same_template()
    {
        static_assert(_IsClassOrUnion<T1>, "same_template works only with classes!");
        return _same_template({ _raw_type_name<T1>(), false }, { _raw_type_name<T2>(), true });
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
        static_assert(_IsClassOrUnion<T1> && _IsClassOrUnion<T2>, "same_template works only with classes!");
        return _same_template({ _raw_type_name<T1>(), false }, { _raw_type_name<T2>(), false });
    }

    // static_assert(same_template<std::array<int, 2>, std::array>());
    // static_assert(!same_template<std::array<int, 2>, std::array<int, 3>>());
    static_assert(same_template<std::char_traits, std::char_traits<char>>());
    static_assert(!same_template<std::char_traits<int>, std::char_traits<char>>());

} // namespace fd
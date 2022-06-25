module;
#include <fd/core/utility.h>

#include <algorithm>
#include <array>
#include <string>

export module fd.type_name;
export import fd.chars_cache;

template <typename T>
constexpr decltype(auto) type_name_raw()
{
    return __FUNCSIG__;
}

template <template <typename...> class T>
constexpr decltype(auto) type_name_raw()
{
    return __FUNCSIG__;
}

class type_name_getter
{
    struct type_name_string : std::string
    {
        constexpr size_t mark_zeros(const std::string_view substr)
        {
            const auto substr_size = substr.size();
            size_t found           = 0;
            size_t pos             = 0;
            for (;;)
            {
                pos = this->find(substr, pos);
                if (pos == this->npos)
                    break;
                ++found;
                std::fill_n(this->data() + pos, substr_size, '\0');
                pos += substr_size;
            }
            return found * substr_size;
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
            size_t found   = 0;
            bool skip_next = false;
            for (auto& c : *this)
            {
                switch (c)
                {
                case ',': {
                    skip_next = true;
                    break;
                }
                case ' ': {
                    if (skip_next)
                        break;
                    c = '\0';
                    ++found;
                }
                default: {
                    skip_next = false;
                    break;
                }
                }
            }

            return found;
        }

        constexpr bool erase_zeros()
        {
            std::string temp;
            for (const auto chr : *this)
            {
                if (chr == '\0')
                    continue;
                temp += chr;
            }
            if (temp.empty())
                return false;
            this->assign(std::move(temp));
            return true;
        }
    };

    std::string buffer_;

  public:
    constexpr type_name_getter(const std::string_view raw_name, const bool is_object)
        : buffer_()
    {
        const auto start = raw_name.find('<') + 1;
        const auto end   = raw_name.rfind('>');
        type_name_string name;
        name.assign(raw_name.data() + start, raw_name.data() + end);

        if (is_object)
        {
            size_t zeros = 0;
            zeros += name.mark_zeros("struct");
            zeros += name.mark_zeros("class");
            zeros += name.mark_zeros("enum");
            zeros += name.mark_zeros("union");
            zeros += name.remove_bad_spaces();
            if (zeros > 0)
                name.erase_zeros();
        }

        buffer_.assign(static_cast<std::string&&>(name));
    }

    constexpr const std::string* operator->() const
    {
        return &buffer_;
    }
};

template <typename T>
constexpr bool class_or_union_v = std::is_class_v<T> || std::is_union_v<T>;

template <typename T>
constexpr auto type_name_holder = [] {
    constexpr std::string_view raw_name = type_name_raw<T>();
    constexpr auto is_object            = class_or_union_v<T> || std::is_enum_v<T>;
    constexpr auto size                 = type_name_getter(raw_name, is_object)->size();
    return fd::chars_cache<char, size + 1>(type_name_getter(raw_name, is_object)->data(), size);
}();

template <template <typename...> class T>
constexpr auto type_name_holder_partial = [] {
    constexpr std::string_view raw_name = type_name_raw<T>();
    constexpr auto size                 = type_name_getter(raw_name, true)->size();
    return fd::chars_cache<char, size + 1>(type_name_getter(raw_name, true)->data(), size);
}();

template <template <typename, size_t> class T>
constexpr auto type_name_holder_partial2 = [] {
    constexpr std::string_view raw_name = type_name_raw<T<int, 1>>();
    constexpr auto size                 = type_name_getter(raw_name, true)->find('<');
    return fd::chars_cache<char, size + 1>(type_name_getter(raw_name, true)->data(), size);
}();

class template_comparer
{
    struct template_info
    {
        const char* name;
        bool partial;
    };

    template_info left_, right_;

  public:
    constexpr template_comparer(const template_info left, const template_info right)
        : left_(left)
        , right_(right)
    {
    }

    constexpr operator bool() const
    {
#if 1
        // every __FUNCSIG__ have own pointer, if they are same - strings are same
        if (left_.name == right_.name)
            return true;
        // we know the strings are different, tempalte part can't be found
        if (left_.partial == right_.partial)
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
            fd::unreachable();
        };

        // skip XXXXtype_name_raw
        const auto offset = std::distance(left_.name, find_existing_char(left_.name, '<')) + 1;

        const auto l_str = left_.name + offset;
        const auto r_str = right_.name + offset;

        const char *partial_str, *full_str;
        if (left_.partial)
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
};

export namespace fd
{
    template <class T>
    constexpr std::string_view type_name()
    {
        return type_name_holder<T>;
    }

    template <template <typename...> class T>
    constexpr std::string_view type_name()
    {
        return type_name_holder_partial<T>;
    }

    // for std::array
    template <template <typename, size_t> class T>
    constexpr std::string_view type_name()
    {
        return type_name_holder_partial2<T>;
    }

    static_assert(type_name<int>() == "int");
    static_assert(type_name<std::char_traits>() == "std::char_traits");
    static_assert(type_name<std::char_traits<char>>() == "std::char_traits<char>");
    static_assert(type_name<std::array>() == "std::array");
    static_assert(type_name<std::exception>() == "std::exception");

    // static_assert(type_name<std::false_type>( ) == "std::integral_constant<bool, false>"); everything is correct but assert fails

    //------------

    /* constexpr std::string drop_namespace(const std::string_view str, const std::string_view drop)
    {
        if (drop.ends_with("::"))
            return erase_substring(str, drop);

        const size_t add = drop.ends_with(':') ? 1 : 2;
        std::string  drop_fixed;
        drop_fixed.reserve(drop.size() + add);
        drop_fixed.assign(drop.begin(), drop.end());
        drop_fixed.resize(drop.size() + add, ':');
        return erase_substring(str, drop_fixed);
    }

    static_assert(drop_namespace("test::test::string", "test") == "string");
    static_assert(drop_namespace("test::test2::string", "test2") == "test::string");
    static_assert(drop_namespace("test::test2::test::string", "test") == "test2::string"); */

    //------------

    template <class T1, template <typename...> class T2>
    constexpr bool same_template()
    {
        static_assert(class_or_union_v<T1>, "same_template works only with classes!");
        return template_comparer({ type_name_raw<T1>(), false }, { type_name_raw<T2>(), true });
    }

    template <template <typename...> class T1, class T2>
    constexpr bool same_template()
    {
        return same_template<T2, T1>();
    }

    template <template <typename...> class T1, template <typename...> class T2>
    constexpr bool same_template()
    {
        return template_comparer({ type_name_raw<T1>(), true }, { type_name_raw<T2>(), true });
    }

    template <class T1, class T2>
    constexpr bool same_template()
    {
        static_assert(class_or_union_v<T1> && class_or_union_v<T2>, "same_template works only with classes!");
        return template_comparer({ type_name_raw<T1>(), false }, { type_name_raw<T2>(), false });
    }

    // static_assert(same_template<std::array<int, 2>, std::array>());
    // static_assert(!same_template<std::array<int, 2>, std::array<int, 3>>());
    static_assert(same_template<std::char_traits, std::char_traits<char>>());
    static_assert(!same_template<std::char_traits<int>, std::char_traits<char>>());

} // namespace fd

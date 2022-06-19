module;
#include <fd/core/utility.h>

#include <algorithm>
#include <array>
#include <string>

export module fd.type_name;

constexpr size_t count_substring(const std::string_view str, const std::string_view substr)
{
    size_t found = 0;
    size_t pos   = 0;
    for (;;)
    {
        pos = str.find(substr, pos);
        if (pos == str.npos)
            break;
        ++found;
        pos += substr.size();
    }
    return found;
}

constexpr void erase_substring(std::string& str, const std::string_view substr)
{
    size_t pos = 0;
    for (;;)
    {
        pos = str.find(substr, pos);
        if (pos == str.npos)
            break;
        str.erase(pos, substr.size());
    }
}

constexpr std::string erase_substring(const std::string_view str, const std::string_view substr, const size_t buffer_size = 0)
{
    std::string result;
    result.reserve(buffer_size == 0 ? str.size() : buffer_size);
    size_t     pos = 0;
    const auto itr = str.begin();
    for (;;)
    {
        size_t from     = pos;
        pos             = str.find(substr, pos);
        const auto done = pos == str.npos;
        result.append(itr + from, done ? str.end() : itr + pos);
        if (done)
            break;
        pos += substr.size();
    }
    return result;
}

#if 0

constexpr size_t override_substring(std::string& str, const std::string_view substr, const char dummy = '\0')
{
	const auto substr_size = substr.size( );
	size_t found = 0;
	size_t pos = 0;
	for(;;)
	{
		pos = str.find(substr, pos);
		if(pos == str.npos)
			break;
		++found;
		std::fill_n(str.data( ) + pos, substr_size, dummy);
		pos += substr_size;
	}
	return found * substr_size;
}

constexpr std::string cleanup_string(const std::string_view str, const size_t buffer_size, const char bad_chr = '\0')
{
	std::string result;
	result.reserve(/*buffer_size == 0 ? str.size( ) :*/ buffer_size);
	for(const auto chr : str)
	{
		if(chr == bad_chr)
			continue;
		result += chr;
	}
	return result;
}

constexpr std::string erase_substring2(const std::string_view str, const std::string_view substr)
{
	std::string temp_str;
	temp_str.assign(str.begin( ), str.end( ));
	const auto buffer_size = override_substring(temp_str, substr, '\0');
	if(buffer_size == 0)
		return temp_str;

	return cleanup_string(temp_str, buffer_size, '\0');
}

#endif

template <typename T>
constexpr T extract_type(const std::string_view raw_name)
{
    const auto start     = raw_name.find('<') + 1;
    const auto end       = raw_name.rfind('>');
    const auto name_size = end - start;
    return {raw_name.data() + start, name_size}; // raw_name.substr(start, name_size), bus string dont accept it
}

constexpr std::array<std::string_view, 4> uselles_words = {"struct ", "class ", "enum ", "union "};

constexpr std::string clean_type_name(const std::string_view raw_name)
{
    auto correct_name = extract_type<std::string>(raw_name);
#if 1
    for (const auto w : uselles_words)
        erase_substring(correct_name, w);
    return correct_name;
#else
    size_t buffer_size = 0;
    for (const auto w : uselles_words)
        buffer_size += override_substring(correct_name, w);
    return buffer_size == 0 ? correct_name : cleanup_string(correct_name, buffer_size);
#endif
}

// returns size without substring
constexpr size_t clean_type_name_size(const std::string_view raw_name)
{
    size_t     removed      = 0;
    const auto correct_name = extract_type<std::string_view>(raw_name);
    for (const auto w : uselles_words)
        removed += count_substring(correct_name, w) * w.size();
    return correct_name.size() - removed;
}

struct template_info
{
    const char* name;
    bool        partial;
};

constexpr bool template_comparer(const template_info left, const template_info right)
{
#if 1
    // every __FUNCSIG__ have own pointer, if they are same - strings are same
    if (left.name == right.name)
        return true;
    // we know the strings are different, tempalte part can't be found
    if (left.partial == right.partial)
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
    const auto offset = std::distance(left.name, find_existing_char(left.name, '<')) + 1;

    const auto l_str = left.name + offset;
    const auto r_str = right.name + offset;

    const char *partial_str, *full_str;
    if (left.partial)
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

template <size_t Size, bool NullTerminated = false>
constexpr auto make_string_buffer(const std::string_view str)
{
    std::array<char, Size + (NullTerminated ? 1 : 0)> buff = {};
#ifndef _DEBUG
    if (buff.size() != str.size())
        buff.fill('\0');
#endif
    std::copy(str.begin(), str.end(), buff.data());
    return buff;
}

template <typename T>
constexpr auto type_name_impl()
{
    constexpr std::string_view raw_name        = type_name_raw<T>();
    constexpr auto             out_buffer_size = clean_type_name_size(raw_name);
    if constexpr (raw_name.size() == out_buffer_size)
        return raw_name;
    else
        return make_string_buffer<out_buffer_size>(clean_type_name(raw_name));
}

template <template <typename...> class T>
constexpr auto type_name_partial_impl()
{
    constexpr std::string_view raw_name        = type_name_raw<T>();
    constexpr auto             out_buffer_size = clean_type_name_size(raw_name);
    if constexpr (raw_name.size() == out_buffer_size)
        return raw_name;
    else
        return make_string_buffer<out_buffer_size>(clean_type_name(raw_name));
}

template <typename T>
constexpr auto type_name_holder = type_name_impl<T>();

template <template <typename...> class T>
constexpr auto type_name_holder_partial = type_name_partial_impl<T>();

template <class T>
constexpr std::string_view extract_holder(const T& holder)
{
    return {holder.data(), holder.size()};
}

export namespace fd
{
    template <class T>
    constexpr std::string_view type_name()
    {
        return extract_holder(type_name_holder<T>);
    }

    template <template <typename...> class T>
    constexpr std::string_view type_name()
    {
        return extract_holder(type_name_holder_partial<T>);
    }

    static_assert(type_name<int>() == "int");
    static_assert(type_name<std::char_traits>() == "std::char_traits");
    static_assert(type_name<std::char_traits<char>>() == "std::char_traits<char>");
    // static_assert(type_name<std::array>() == "std::array");
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
        static_assert(std::is_class_v<T1>, "same_template works only with classes!");
        return template_comparer({type_name_raw<T1>(), false}, {type_name_raw<T2>(), true});
    }

    template <template <typename...> class T1, class T2>
    constexpr bool same_template()
    {
        return same_template<T2, T1>();
    }

    template <template <typename...> class T1, template <typename...> class T2>
    constexpr bool same_template()
    {
        return template_comparer({type_name_raw<T1>(), true}, {type_name_raw<T2>(), true});
    }

    template <class T1, class T2>
    constexpr bool same_template()
    {
        static_assert(std::is_class_v<T1> && std::is_class_v<T2>, "same_template works only with classes!");
        return template_comparer({type_name_raw<T1>(), false}, {type_name_raw<T2>(), false});
    }

    // static_assert(same_template<std::array<int, 2>, std::array>());
    // static_assert(!same_template<std::array<int, 2>, std::array<int, 3>>());
    static_assert(same_template<std::char_traits, std::char_traits<char>>());
    static_assert(!same_template<std::char_traits<int>, std::char_traits<char>>());

} // namespace fd

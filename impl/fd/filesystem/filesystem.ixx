module;

#include <cstdint>

export module fd.filesystem;
export import fd.filesystem.path;

using fd::basic_string;
using fd::basic_string_view;

enum class funcs
{
    dir_create,
    // file_create,
    is_directory,
    is_file,
    dir_empty,
    file_empty
};

class function_selector
{
    funcs fn_tag_;
    bool error_ = false;

  public:
    function_selector(const funcs tag)
        : fn_tag_(tag)
    {
    }

    bool operator()(const char8_t* str);
    bool operator()(const char* str);
    bool operator()(const wchar_t* str);
    bool operator()(const char16_t* str);
    bool operator()(const char32_t* str);

    operator bool() const;
};

template <class C, typename Fn>
bool _Safe_legacy_string_call(const C* str, Fn fn)
{
    return fn(str);
}

template <class C, typename Fn>
bool _Safe_legacy_string_call(const basic_string<C>& str, Fn fn)
{
    return fn(str.data());
}

template <class C, typename Fn>
bool _Safe_legacy_string_call(const basic_string_view<C>& str, Fn fn)
{
    const auto begin = str.data();
    const auto end   = begin + str.size();
    if (*end == '\0')
        return fn(begin);

    const basic_string buff(begin, end);
    return fn(buff.data());
}

template <funcs Tag>
struct invoker
{
    template <typename C>
    bool operator()(const C* str) const
    {
        return _Safe_legacy_string_call(str, function_selector(Tag));
    }

    template <typename C>
    bool operator()(const basic_string<C>& str) const
    {
        return _Safe_legacy_string_call(str, function_selector(Tag));
    }

    template <typename C>
    bool operator()(const basic_string_view<C> str) const
    {
        return _Safe_legacy_string_call(str, function_selector(Tag));
    }
};

struct create_directories_impl
{
    template <typename C>
    using _Path = fd::fs::basic_path<C, basic_string_view>;

    template <typename C>
    bool operator()(const _Path<C> path) const
    {
        // buffer added because system use legacy strings
        basic_string<C> buff;
        buff.reserve(path.size());

#ifdef _DEBUG
        size_t created = 0;
#else
        auto created = false;
#endif
        const auto try_create = [&](const size_t offset) -> uint8_t {
            const auto bg = path.data() + buff.size() - 1;
            const auto ed = &path[offset];
            buff.append(bg, ed - 1); //- 1 for push_back
#ifdef _WIN32
            buff.push_back('\\');
#else
            buff.push_back('/');
#endif

            function_selector dir_exists(funcs::is_directory);
            if (dir_exists(buff.data()))
                return 2;
            if (!dir_exists)
                return 0;

            function_selector creator(funcs::dir_create);
            if (!creator(buff.data()))
                return 0;
            if (!creator)
                return 0;

#ifdef _DEBUG
            ++created;
#else
            created = true;
#endif
            return 1;
        };

        // copypasted from path
        constexpr auto _Is_slash = [](const C chr) {
            return chr == '\\' || chr == '/';
        };

        for (auto offset = path.root_path().size(); offset < path.size(); ++offset)
        {
            if (!_Is_slash(path[offset]))
                continue;

            const auto check = try_create(offset);
            if (!check)
                return false;
            if (check == 2)
                return created;
        }

        if (!_Is_slash(path.back()))
        {
            const auto check = try_create(path.size() - 1);
            if (!check)
                return false;
        }

        return created;
    }

    template <typename C>
    bool operator()(const basic_string<C>& path) const
    {
        return operator()(_Path<C>(path.begin(), path.end()));
    }

    template <typename C>
    bool operator()(const C* path) const
    {
        return operator()(_Path<C>(path));
    }
};

export namespace fd::fs
{
    constexpr invoker<funcs::dir_create> create_directory;
    constexpr invoker<funcs::is_directory> is_directory;
    constexpr invoker<funcs::is_file> is_file;
    constexpr invoker<funcs::dir_empty> directory_empty;
    constexpr invoker<funcs::file_empty> file_empty;

    constexpr create_directories_impl create_directories;
} // namespace fd::fs

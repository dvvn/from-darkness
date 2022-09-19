module;

#include <cstdint>

export module fd.filesystem;
export import fd.filesystem.path;

namespace fd::fs
{
    enum class funcs : uint8_t
    {
        dir_create,
        // file_create,
        is_directory,
        is_file,
        dir_empty,
        file_empty
    };

    class basic_function_selector
    {
        funcs fn_tag_;

      public:
        constexpr basic_function_selector(const funcs tag)
            : fn_tag_(tag)
        {
        }

        bool operator()(const char8_t* str) const;
        bool operator()(const char* str) const;
        bool operator()(const wchar_t* str) const;
        bool operator()(const char16_t* str) const;
        bool operator()(const char32_t* str) const;
    };

    class function_selector
    {
        funcs fn_tag_;
        bool error_;

      public:
        function_selector(const funcs tag);
        bool operator()(const char8_t* str);
        bool operator()(const char* str);
        bool operator()(const wchar_t* str);
        bool operator()(const char16_t* str);
        bool operator()(const char32_t* str);
        operator bool() const;
    };

    template <funcs Tag>
    class invoker
    {
        basic_function_selector fn_ = Tag;

      public:
        template <typename C>
        bool operator()(const C* str) const
        {
            return fn_(str);
        }

        template <typename C>
        bool operator()(const basic_string<C>& str) const
        {
            return fn_(str.data());
        }

        template <typename C>
        bool operator()(const basic_string_view<C> str) const
        {
            bool ret;
            const auto begin = str.data();
            const auto end   = begin + str.size();
            if (*end == '\0')
                ret = fn_(begin);
            else
                ret = fn_(basic_string<C>(begin, end).data());
            return ret;
        }
    };

    export constexpr invoker<funcs::dir_create> create_directory;
    export constexpr invoker<funcs::is_directory> is_directory;
    export constexpr invoker<funcs::is_file> is_file;
    export constexpr invoker<funcs::dir_empty> directory_empty;
    export constexpr invoker<funcs::file_empty> file_empty;

    struct create_directories_impl
    {
        template <typename C>
        using _Path = basic_path<C, basic_string_view>;

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

                function_selector dir_exists = funcs::is_directory;
                if (dir_exists(buff.data()))
                    return 2;
                if (!dir_exists)
                    return 0;

                function_selector creator = funcs::dir_create;
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

    export constexpr create_directories_impl create_directories;
} // namespace fd::fs

module;

#include <cstdint>

export module fd.filesystem;
export import fd.filesystem.path;

namespace fd::fs
{
    struct directory_impl
    {
        bool operator()(const wstring_view dir) const;
        bool operator()(const string_view dir) const;

        bool create(const wstring_view dir) const;
        bool create(const string_view dir) const;

        bool empty(const wstring_view dir) const;
        bool empty(const string_view dir) const;
    };

    export constexpr directory_impl directory;

    struct file_impl
    {
        bool operator()(const wstring_view dir) const;
        bool operator()(const string_view dir) const;
    };

    export constexpr file_impl file;
} // namespace fd::fs
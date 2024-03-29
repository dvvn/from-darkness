#pragma once

#include <fd/algorithm.h>
#include <fd/string.h>

#include <algorithm>

namespace fd
{
template <typename C, template <typename> class Buff>
class _fs_buffer
{
    using buff_type = Buff<C>;

    /*using range_type    = range_view<const C*>;
    using str_view_type = basic_string_view<C>;
    using str_type      = basic_string<C>;*/

    buff_type buff_;

  public:
    constexpr _fs_buffer(buff_type buff = {})
        : buff_(std::move(buff))
    {
    }

    constexpr auto begin() const
    {
        return _begin(buff_);
    }

    constexpr auto end() const
    {
        return _end(buff_);
    }

    constexpr auto begin()
    {
        return _begin(buff_);
    }

    constexpr auto end()
    {
        return _end(buff_);
    }

    //---
};

template <typename C>
static constexpr bool _is_slash(const C chr)
{
    return chr == '\\' || chr == '/';
}

template <typename T>
static constexpr range_view<iter_t<T>> _file_name(T& file)
{
    const reverse_iterator end = _begin(file);
    for (reverse_iterator it = _end(file); it != end; ++it)
    {
        if (_is_slash(*it))
            return { it.base(), _end(file) };
    }

    return {};
}

template <typename T>
static constexpr range_view<iter_t<T>> _file_extension(T& file)
{
    const reverse_iterator end = _begin(file);
    for (reverse_iterator it = _end(file); it != end; ++it)
    {
        if (_is_slash(*it))
            break; // filename without extension
        if (*it == '.')
        {
            auto next = it;
            ++next;
            if (next == end)
                break; // filename without extension
            if (_is_slash(*next))
                break; // filename starts from dot
            return { it.base(), _end(file) };
        }
    }

    return {};
}

bool _file_exists(const char* begin, const char* end);
bool _file_exists(const wchar_t* begin, const wchar_t* end);

bool _file_empty(const char* begin, const char* end);
bool _file_empty(const wchar_t* begin, const wchar_t* end);

template <typename C>
class file_view : public _fs_buffer<C, basic_string_view>
{
  public:
    using _fs_buffer<C, basic_string_view>::_fs_buffer;

    constexpr auto name() const
    {
        return _file_name(*this);
    }

    constexpr auto extension() const
    {
        return _file_extension(*this);
    }

    bool exists() const
    {
        return _file_exists(this->begin(), this->end());
    }
};

template <typename T>
file_view(T&&) -> file_view<std::iter_value_t<iter_t<T>>>;

template <typename C>
class file : public _fs_buffer<C, basic_string>
{
  public:
    using _fs_buffer<C, basic_string>::_fs_buffer;

    auto name() const
    {
        return _file_name(*this);
    }

    auto extension() const
    {
        return _file_extension(*this);
    }

    bool exists() const
    {
        return _file_exists(this->begin(), this->end());
    }
};

template <typename T>
file(T&&) -> file<std::iter_value_t<iter_t<T>>>;

bool _directory_create(const char* begin, const char* end);
bool _directory_create(const wchar_t* begin, const wchar_t* end);

bool _directory_empty(const char* begin, const char* end);
bool _directory_empty(const wchar_t* begin, const wchar_t* end);

template <typename C>
class directory_view : public _fs_buffer<C, basic_string_view>
{
  public:
    using _fs_buffer<C, basic_string_view>::_fs_buffer;

    bool create()
    {
        return _directory_create(this->begin(), this->end());
    }

    bool empty() const
    {
        return _directory_empty(this->begin(), this->end());
    }
};

template <typename T>
directory_view(T&&) -> directory_view<std::iter_value_t<iter_t<T>>>;

template <typename C>
class directory : public _fs_buffer<C, basic_string>
{
  public:
    using _fs_buffer<C, basic_string>::_fs_buffer;

    bool create()
    {
        return _directory_create(this->begin(), this->end());
    }

    bool empty() const
    {
        return _directory_empty(this->begin(), this->end());
    }
};

template <typename T>
directory(T&&) -> directory<std::iter_value_t<iter_t<T>>>;

} // namespace fd

#if 0
namespace fd::fs
{
// copypasted from std::filesystem
#pragma region copypasted

// ReSharper disable once CppInconsistentNaming
static constexpr auto _Is_slash = [](const auto chr) {
    return chr == '\\' || chr == '/';
};

template <typename Itr>
static constexpr bool _Is_drive_prefix(Itr first)
{
    return /* first[1] */ *++first == ':';
}

template <typename Itr>
static constexpr bool _Has_drive_letter_prefix(const Itr first, const Itr last)
{
    // test if [first, last) has a prefix of the form X:
    return std::distance(first, last) >= 2 && _Is_drive_prefix(first);
}

template <typename Itr>
static constexpr auto _Unwrap_iter(const Itr itr)
{
#ifdef _MSC_VER
    if constexpr (std::_Unwrappable_v<Itr>)
        return itr._Unwrapped();
#else

#endif

    return &*itr;
}

template <typename Itr>
static constexpr Itr _Find_root_name_end(const Itr first, const Itr last)
{
    if constexpr (std::is_class_v<Itr>)
    {
        const auto first1 = _Unwrap_iter(first);
        const auto last1  = _Unwrap_iter(last);
        const auto found  = _Find_root_name_end(first1, last1);
        return first + std::distance(first1, found);
    }

    // attempt to parse [first, last) as a path and return the end of root-name if it exists; otherwise, first

    // This is the place in the generic grammar where library implementations have the most freedom.
    // Below are example Windows paths, and what we've decided to do with them:
    // * X:DriveRelative, X:\DosAbsolute
    //   We parse X: as root-name, if and only if \ is present we consider that root-directory
    // * \RootRelative
    //   We parse no root-name, and \ as root-directory
    // * \\server\share
    //   We parse \\server as root-name, \ as root-directory, and share as the first element in relative-path.
    //   Technically, Windows considers all of \\server\share the logical "root", but for purposes
    //   of decomposition we want those split, so that path(R"(\\server\share)").replace_filename("other_share")
    //   is \\server\other_share
    // * \\?\device
    // * \??\device
    // * \\.\device
    //   CreateFile appears to treat these as the same thing; we will set the first three characters as root-name
    //   and the first \ as root-directory. Support for these prefixes varies by particular Windows version, but
    //   for the purposes of path decomposition we don't need to worry about that.
    // * \\?\UNC\server\share
    //   MSDN explicitly documents the \\?\UNC syntax as a special case. What actually happens is that the device
    //   Mup, or "Multiple UNC provider", owns the path \\?\UNC in the NT namespace, and is responsible for the
    //   network file access. When the user says \\server\share, CreateFile translates that into
    //   \\?\UNC\server\share to get the remote server access behavior. Because NT treats this like any other
    //   device, we have chosen to treat this as the \\?\ case above.
    const size_t diff = std::distance(first, last);
    if (diff < 2)
    {
        return first;
    }

    if (_Has_drive_letter_prefix(first, last))
    {
        // check for X: first because it's the most common root-name
        return first + 2;
    }

    if (!_Is_slash(first[0]))
    {
        // all the other root-names start with a slash; check that first because
        // we expect paths without a leading slash to be very common
        return first;
    }

    // $ means anything other than a slash, including potentially the end of the input
    if (diff >= 4 && _Is_slash(first[3]) && (diff == 4 || !_Is_slash(first[4])) // \xx\$
        && (_Is_slash(first[1]) && (first[2] == '?' || first[2] == '.')         // \\?\$ or \\.\$
            || first[1] == '?' && first[2] == '?'))                             // \??\$
    {
        return first + 3;
    }

    if (diff >= 3 && _Is_slash(first[1]) && !_Is_slash(first[2])) // \\server
    {
        return std::find_if(first + 3, last, _Is_slash);
    }

    // no match
    return first;
}

template <typename Itr>
static constexpr Itr _Find_relative_path(const Itr first, const Itr last)
{
    // attempt to parse [first, last) as a path and return the start of relative-path
    return std::find_if_not(_Find_root_name_end(first, last), last, _Is_slash);
}

template <typename Itr>
static constexpr Itr _Find_filename(const Itr first, Itr last)
{
    // attempt to parse [first, last) as a path and return the start of filename if it exists; otherwise, last

    for (const auto relPath = _Find_relative_path(first, last); relPath != last;) // while (relPath != last && !_Is_slash(last[-1]))
    {
        auto backItr = last - 1;
        if (_Is_slash(*backItr))
            break;
        last = std::move(backItr);
    }
    return last;
}

template <typename Itr>
static constexpr Itr _Find_extension(const Itr fname, const Itr ads)
{
    // offset_to dividing point between stem and extension in a generic format filename consisting of [fname, ads)
    auto ext = ads;
    if (fname == ext)
    {
        // empty path
        return ads;
    }

    --ext;
    if (fname == ext)
    {
        // path is length 1 and either dot, or has no dots; either way, extension() is empty
        return ads;
    }

    if (*ext == '.')
    {
        const auto extPrev = ext - 1;
        // we might have found the end of stem
        if (fname == extPrev && *extPrev == '.')
        {
            // dotdot special case
            return ads;
        }
        else
        {
            // x.
            return ext;
        }
    }

    while (fname != --ext)
    {
        if (*ext == '.')
        {
            // found a dot which is not in first position, so it starts extension()
            return ext;
        }
    }

    // if we got here, either there are no dots, in which case extension is empty, or the first element
    // is a dot, in which case we have the leading single dot special case, which also makes extension empty
    return ads;
}

#pragma endregion

template <typename C, template <typename...> class S>
struct extension : S<C>
{
    using S<C>::S;
};

template <typename C, template <typename...> class S>
class filename : public S<C>
{
    using str_t = S<C>;

    bool trimmed_;

    struct ext_data
    {
        using pointer = const C*;
        pointer fname, ads, ext;
    };

    constexpr ext_data _Ext() const
    {
        const auto first = str_t::data();
        const auto last  = first + str_t::size();
        const auto fname = trimmed_ ? first : _Find_filename(first, last);
        const auto ads   = std::find(fname, last, ':'); // strip alternate data streams in intra-filename decomposition
        const auto ext   = _Find_extension(fname, ads);
        return { fname, ads, ext };
    }

  public:
    template <typename... Ts>
    constexpr filename(const bool trimmed, const Ts... args)
        : str_t(args...)
    {
        trimmed_ = trimmed;
    }

    // stripped of its extension
    constexpr str_t stem() const
    {
        // attempt to parse text_ as a path and return the stem if it exists; otherwise, an empty view
        const auto [fname, ads, ext] = _Ext();
        return { fname, static_cast<size_t>(ext - fname) };
    }

    constexpr extension<C, S> extension() const
    {
        // attempt to parse text_ as a path and return the extension if it exists; otherwise, an empty view
        const auto [fname, ads, ext] = _Ext();
        return { ext, static_cast<size_t>(ads - ext) };
    }
};

template <typename C, template <typename...> class S>
class basic_path : public S<C>
{
    using str_t = S<C>;

  public:
    template <typename... Args>
    constexpr basic_path(Args&&... args)
        : str_t(std::forward<Args>(args)...)
    {
    }

    constexpr str_t root_name() const
    {
        // attempt to parse text_ as a basic_path and return the root-name if it exists; otherwise, an empty view
        const auto first = str_t::data();
        const auto last  = first + str_t::size();
        return { first, _Find_root_name_end(first, last) };
    }

    constexpr str_t root_directory() const
    {
        // attempt to parse text_ as a basic_path and return the root-directory if it exists; otherwise, an empty view
        const auto first       = str_t::data();
        const auto last        = first + str_t::size();
        const auto rootNameEnd = _Find_root_name_end(first, last);
        const auto relPath     = std::find_if_not(rootNameEnd, last, _Is_slash);
        return { rootNameEnd, relPath };
    }

    constexpr str_t root_path() const
    {
        // attempt to parse text_ as a basic_path and return the root-basic_path if it exists; otherwise, an empty view
        const auto first = str_t::data();
        const auto last  = first + str_t::size();
        return { first, _Find_relative_path(first, last) };
    };

    constexpr basic_path relative_path() const
    {
        // attempt to parse text_ as a basic_path and return the relative-basic_path if it exists; otherwise, an empty view
        const auto first   = str_t::data();
        const auto last    = first + str_t::size();
        const auto relPath = _Find_relative_path(first, last);
        return { relPath, last };
    }

    constexpr basic_path parent_path() const
    {
        // attempt to parse text_ as a basic_path and return the parent_path if it exists; otherwise, an empty view
        const auto first   = str_t::data();
        auto       last    = first + str_t::size();
        const auto relPath = _Find_relative_path(first, last);
        // case 1: relative-basic_path ends in a directory-separator, remove the separator to remove "magic empty basic_path"
        //  for example: R"(/cat/dog/\//\)"
        // case 2: relative-basic_path doesn't end in a directory-separator, remove the filename and last directory-separator
        //  to prevent creation of a "magic empty basic_path"
        //  for example: "/cat/dog"
        while (relPath != last && !_Is_slash(last[-1]))
        {
            // handle case 2 by removing trailing filename, puts us into case 1
            --last;
        }

        while (relPath != last && _Is_slash(last[-1]))
        {
            // handle case 1 by removing trailing slashes
            --last;
        }

        return { first, last };
    }

  private:
    constexpr filename<C, basic_string_view> filename_view() const
    {
        return { false, str_t::data(), str_t::size() };
    }

  public:
    constexpr filename<C, S> filename() const
    {
        // attempt to parse text_ as a basic_path and return the filename if it exists; otherwise, an empty view
        const auto first = str_t::data();
        const auto last  = first + str_t::size();
        const auto fname = _Find_filename(first, last);
        return { true, fname, last };
    }

    constexpr str_t stem() const
    {
        return filename_view().stem();
    }

    constexpr str_t extension() const
    {
        return filename_view().extension();
    }
};

template <typename C>
using path = basic_path<C, basic_string>;

template <typename C>
using path_view = basic_path<C, basic_string_view>;

template <typename C>
basic_path(const C*) -> basic_path<C, basic_string_view>;
template <typename C>
basic_path(basic_string_view<C>) -> basic_path<C, basic_string_view>;
template <typename C>
basic_path(const basic_string<C>&) -> basic_path<C, basic_string_view>;
template <typename C>
basic_path(basic_string<C>&&) -> basic_path<C, basic_string>;

//---------------

struct directory_impl
{
    bool operator()(wstring_view dir) const;
    bool operator()(string_view dir) const;

    bool create(wstring_view dir, bool override) const;
    bool create(string_view dir, bool override) const;

    bool empty(wstring_view dir) const;
    bool empty(string_view dir) const;
};

constexpr directory_impl Directory;

struct file_impl
{
    bool operator()(wstring_view dir) const;
    bool operator()(string_view dir) const;

    bool create(wstring_view dir, bool override) const;
    bool create(string_view dir, bool override) const;
};

constexpr file_impl File;
} // namespace fd::fs
#endif
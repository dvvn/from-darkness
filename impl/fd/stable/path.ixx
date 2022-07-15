module;

#include <algorithm>
#include <string_view>

export module fd.path;

// copypasted from std::filesystem

#pragma region copypasted

constexpr auto _Is_slash = []<typename C>(const C chr) {
    return chr == static_cast<C>('\\') || chr == static_cast<C>('/');
};

template <typename C>
constexpr bool _Is_drive_prefix(const C* const first)
{
    return first[1] == static_cast<C>(':');
}

template <typename C>
constexpr bool _Has_drive_letter_prefix(const C* const first, const C* const last)
{
    // test if [first, last) has a prefix of the form X:
    return last - first >= 2 && _Is_drive_prefix(first);
}

template <typename C>
constexpr const C* _Find_root_name_end(const C* const first, const C* const last)
{
    // attempt to parse [first, last) as a path_impl and return the end of root-name if it exists; otherwise, first

    // This is the place in the generic grammar where library implementations have the most freedom.
    // Below are example Windows paths, and what we've decided to do with them:
    // * X:DriveRelative, X:\DosAbsolute
    //   We parse X: as root-name, if and only if \ is present we consider that root-directory
    // * \RootRelative
    //   We parse no root-name, and \ as root-directory
    // * \\server\share
    //   We parse \\server as root-name, \ as root-directory, and share as the first element in relative-path_impl.
    //   Technically, Windows considers all of \\server\share the logical "root", but for purposes
    //   of decomposition we want those split, so that path_impl(R"(\\server\share)").replace_filename("other_share")
    //   is \\server\other_share
    // * \\?\device
    // * \??\device
    // * \\.\device
    //   CreateFile appears to treat these as the same thing; we will set the first three characters as root-name
    //   and the first \ as root-directory. Support for these prefixes varies by particular Windows version, but
    //   for the purposes of path_impl decomposition we don't need to worry about that.
    // * \\?\UNC\server\share
    //   MSDN explicitly documents the \\?\UNC syntax as a special case. What actually happens is that the device
    //   Mup, or "Multiple UNC provider", owns the path_impl \\?\UNC in the NT namespace, and is responsible for the
    //   network file access. When the user says \\server\share, CreateFile translates that into
    //   \\?\UNC\server\share to get the remote server access behavior. Because NT treats this like any other
    //   device, we have chosen to treat this as the \\?\ case above.
    if (last - first < 2)
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
    if (last - first >= 4 && _Is_slash(first[3]) && (last - first == 4 || !_Is_slash(first[4]))           // \xx\$
        && ((_Is_slash(first[1]) && (first[2] == static_cast<C>('?') || first[2] == static_cast<C>('.'))) // \\?\$ or \\.\$
            || (first[1] == static_cast<C>('?') && first[2] == static_cast<C>('?'))))                     // \??\$
    {
        return first + 3;
    }

    if (last - first >= 3 && _Is_slash(first[1]) && !_Is_slash(first[2])) // \\server
    {
        return std::find_if(first + 3, last, _Is_slash);
    }

    // no match
    return first;
}

template <typename C>
constexpr const C* _Find_relative_path(const C* const first, const C* const last)
{
    // attempt to parse [first, last) as a path_impl and return the start of relative-path_impl
    return std::find_if_not(_Find_root_name_end(first, last), last, _Is_slash);
}

template <typename C>
constexpr const C* _Find_filename(const C* const first, const C* last)
{
    // attempt to parse [first, last) as a path_impl and return the start of filename if it exists; otherwise, last
    const auto rel_path = _Find_relative_path(first, last);
    while (rel_path != last && !_Is_slash(last[-1]))
    {
        --last;
    }

    return last;
}

template <typename C>
constexpr const C* _Find_extension(const C* const fname, const C* const ads)
{
    // find dividing point between stem and extension in a generic format filename consisting of [fname, ads)
    auto ext = ads;
    if (fname == ext)
    {
        // empty path_impl
        return ads;
    }

    --ext;
    if (fname == ext)
    {
        // path_impl is length 1 and either dot, or has no dots; either way, extension() is empty
        return ads;
    }

    if (*ext == static_cast<C>('.'))
    {
        // we might have found the end of stem
        if (fname == ext - 1 && ext[-1] == static_cast<C>('.'))
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
        if (*ext == static_cast<C>('.'))
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

template <typename C>
class extension : public std::basic_string_view<C>
{
    using _View = std::basic_string_view<C>;

  public:
    using _View::_View;

    /* constexpr bool is_header() const
    {
        switch (_View::operator[](1))
        {
        case static_cast<C>('h'): //.h**
#if __cplusplus >= 202002L
        case static_cast<C>('i'): //.i**
#endif
            return true;
        default:
            return false;
        }
    }

    constexpr bool is_source() const
    {
        return _View::operator[](1) == static_cast<C>('c');
    } */
};

template <typename C>
class filename : public std::basic_string_view<C>
{
    using _View = std::basic_string_view<C>;

    bool trimmed_;

    struct ext_data
    {
        using pointer = const C*;
        pointer fname, ads, ext;
    };

    constexpr ext_data _Ext() const
    {
        const auto first = _View::data();
        const auto last  = first + _View::size();
        const auto fname = trimmed_ ? first : _Find_filename(first, last);
        const auto ads   = std::find(fname, last, static_cast<C>(':')); // strip alternate data streams in intra-filename decomposition
        const auto ext   = _Find_extension(fname, ads);
        return { fname, ads, ext };
    }

  public:
    template <typename... Ts>
    constexpr filename(const bool trimmed, const Ts... args)
        : _View(args...)
    {
        trimmed_ = trimmed;
    }

    // stripped of its extension
    constexpr _View stem() const
    {
        // attempt to parse text_ as a path_impl and return the stem if it exists; otherwise, an empty view
        const auto [fname, ads, ext] = _Ext();
        return { fname, static_cast<size_t>(ext - fname) };
    }

    constexpr ::extension<C> extension() const
    {
        // attempt to parse text_ as a path_impl and return the extension if it exists; otherwise, an empty view
        const auto [fname, ads, ext] = _Ext();
        return { ext, static_cast<size_t>(ads - ext) };
    }
};

template <typename T, typename... Ts>
filename(bool, const T, Ts...) -> filename<std::iter_value_t<T>>;

template <typename C>
class path_impl : public std::basic_string_view<C>
{
    using _View = std::basic_string_view<C>;

  public:
    template <typename... Args>
    constexpr path_impl(const Args&... args)
        : _View(args...)
    {
    }

    constexpr _View root_name() const
    {
        // attempt to parse text_ as a path_impl and return the root-name if it exists; otherwise, an empty view
        const auto first = _View::data();
        const auto last  = first + _View::size();
        return { first, static_cast<size_t>(_Find_root_name_end(first, last) - first) };
    }

    constexpr _View root_directory() const
    {
        // attempt to parse text_ as a path_impl and return the root-directory if it exists; otherwise, an empty view
        const auto first         = _View::data();
        const auto last          = first + _View::size();
        const auto root_name_end = _Find_root_name_end(first, last);
        const auto rel_path      = std::find_if_not(root_name_end, last, _Is_slash);
        return { root_name_end, static_cast<size_t>(rel_path - root_name_end) };
    }

    constexpr _View root_path() const
    {
        // attempt to parse text_ as a path_impl and return the root-path_impl if it exists; otherwise, an empty view
        const auto first = _View::data();
        const auto last  = first + _View::size();
        return { first, static_cast<size_t>(_Find_relative_path(first, last) - first) };
    };

    constexpr path_impl relative_path() const
    {
        // attempt to parse text_ as a path_impl and return the relative-path_impl if it exists; otherwise, an empty view
        const auto first    = _View::data();
        const auto last     = first + _View::size();
        const auto rel_path = _Find_relative_path(first, last);
        return { rel_path, static_cast<size_t>(last - rel_path) };
    }

    constexpr path_impl parent_path() const
    {
        // attempt to parse text_ as a path_impl and return the parent_path if it exists; otherwise, an empty view
        const auto first    = _View::data();
        auto last           = first + _View::size();
        const auto rel_path = _Find_relative_path(first, last);
        // case 1: relative-path_impl ends in a directory-separator, remove the separator to remove "magic empty path_impl"
        //  for example: R"(/cat/dog/\//\)"
        // case 2: relative-path_impl doesn't end in a directory-separator, remove the filename and last directory-separator
        //  to prevent creation of a "magic empty path_impl"
        //  for example: "/cat/dog"
        while (rel_path != last && !_Is_slash(last[-1]))
        {
            // handle case 2 by removing trailing filename, puts us into case 1
            --last;
        }

        while (rel_path != last && _Is_slash(last[-1]))
        {
            // handle case 1 by removing trailing slashes
            --last;
        }

        return { first, static_cast<size_t>(last - first) };
    }

    constexpr filename<C> filename() const
    {
        // attempt to parse text_ as a path_impl and return the filename if it exists; otherwise, an empty view
        const auto first = _View::data();
        const auto last  = first + _View::size();
        const auto fname = _Find_filename(first, last);
        return { true, fname, static_cast<size_t>(last - fname) };
    }

    constexpr auto stem() const
    {
        return ::filename(false, _View::data(), _View::size()).stem();
    }

    constexpr auto extension() const
    {
        return ::filename(false, _View::data(), _View::size()).extension();
    }
};

template <typename C>
struct path;

template <typename C>
path(const C*) -> path<C>;
template <typename C, typename Tr>
path(const std::basic_string_view<C, Tr>) -> path<C>;
/* template <typename C, class Al>
path(const std::basic_string<C, std::char_traits<C>, Al>&) -> path<C>; */

#define PATH_IMPL(_T_)                      \
    template <>                             \
    struct path<_T_> : path_impl<_T_>       \
    {                                       \
        template <typename... Args>         \
        constexpr path(const Args&... args) \
            : path_impl<_T_>(args...)       \
        {                                   \
        }                                   \
    };

PATH_IMPL(char);
PATH_IMPL(wchar_t);
PATH_IMPL(char8_t);
PATH_IMPL(char16_t);
PATH_IMPL(char32_t);

export namespace fd
{
    using ::path;
}

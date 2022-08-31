module;

#include <algorithm>

export module fd.filesystem.path;
export import fd.string;

using fd::basic_string;
using fd::basic_string_view;

// copypasted from std::filesystem

#pragma region copypasted

constexpr auto _Is_slash = []<typename C>(const C chr) {
    return chr == '\\' || chr == '/';
};

template <typename Itr>
constexpr bool _Is_drive_prefix(Itr first)
{
    return /* first[1] */ *++first == ':';
}

template <typename Itr>
constexpr bool _Has_drive_letter_prefix(const Itr first, const Itr last)
{
    // test if [first, last) has a prefix of the form X:
    return std::distance(first, last) >= 2 && _Is_drive_prefix(first);
}

template <typename Itr>
constexpr auto _Unwrap_iter(const Itr itr)
{
#ifdef _MSC_VER
    if constexpr (std::_Unwrappable_v<Itr>)
        return itr._Unwrapped();
#else

#endif

    return &*itr;
}

template <typename Itr>
constexpr Itr _Find_root_name_end(const Itr first, const Itr last)
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
        && ((_Is_slash(first[1]) && (first[2] == '?' || first[2] == '.'))       // \\?\$ or \\.\$
            || (first[1] == '?' && first[2] == '?')))                           // \??\$
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
constexpr Itr _Find_relative_path(const Itr first, const Itr last)
{
    // attempt to parse [first, last) as a path and return the start of relative-path
    return std::find_if_not(_Find_root_name_end(first, last), last, _Is_slash);
}

template <typename Itr>
constexpr Itr _Find_filename(const Itr first, Itr last)
{
    // attempt to parse [first, last) as a path and return the start of filename if it exists; otherwise, last
    const auto rel_path = _Find_relative_path(first, last);
    for (;;) // while (rel_path != last && !_Is_slash(last[-1]))
    {
        if (rel_path == last)
            break;
        auto back_itr = last - 1;
        if (_Is_slash(*back_itr))
            break;
        last = std::move(back_itr);
    }
    return last;
}

template <typename Itr>
constexpr Itr _Find_extension(const Itr fname, const Itr ads)
{
    // find dividing point between stem and extension in a generic format filename consisting of [fname, ads)
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
        const auto ext_prev = ext - 1;
        // we might have found the end of stem
        if (fname == ext_prev && *ext_prev == '.')
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
    using _Str = S<C>;

    bool trimmed_;

    struct ext_data
    {
        using pointer = const C*;
        pointer fname, ads, ext;
    };

    constexpr ext_data _Ext() const
    {
        const auto first = _Str::data();
        const auto last  = first + _Str::size();
        const auto fname = trimmed_ ? first : _Find_filename(first, last);
        const auto ads   = std::find(fname, last, ':'); // strip alternate data streams in intra-filename decomposition
        const auto ext   = _Find_extension(fname, ads);
        return { fname, ads, ext };
    }

  public:
    template <typename... Ts>
    constexpr filename(const bool trimmed, const Ts... args)
        : _Str(args...)
    {
        trimmed_ = trimmed;
    }

    // stripped of its extension
    constexpr _Str stem() const
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
    using _Str = S<C>;

  public:
    template <typename... Args>
    constexpr basic_path(Args&&... args)
        : _Str(std::forward<Args>(args)...)
    {
    }

    constexpr _Str root_name() const
    {
        // attempt to parse text_ as a basic_path and return the root-name if it exists; otherwise, an empty view
        const auto first = _Str::data();
        const auto last  = first + _Str::size();
        return { first, _Find_root_name_end(first, last) };
    }

    constexpr _Str root_directory() const
    {
        // attempt to parse text_ as a basic_path and return the root-directory if it exists; otherwise, an empty view
        const auto first         = _Str::data();
        const auto last          = first + _Str::size();
        const auto root_name_end = _Find_root_name_end(first, last);
        const auto rel_path      = std::find_if_not(root_name_end, last, _Is_slash);
        return { root_name_end, rel_path };
    }

    constexpr _Str root_path() const
    {
        // attempt to parse text_ as a basic_path and return the root-basic_path if it exists; otherwise, an empty view
        const auto first = _Str::data();
        const auto last  = first + _Str::size();
        return { first, _Find_relative_path(first, last) };
    };

    constexpr basic_path relative_path() const
    {
        // attempt to parse text_ as a basic_path and return the relative-basic_path if it exists; otherwise, an empty view
        const auto first    = _Str::data();
        const auto last     = first + _Str::size();
        const auto rel_path = _Find_relative_path(first, last);
        return { rel_path, last };
    }

    constexpr basic_path parent_path() const
    {
        // attempt to parse text_ as a basic_path and return the parent_path if it exists; otherwise, an empty view
        const auto first    = _Str::data();
        auto last           = first + _Str::size();
        const auto rel_path = _Find_relative_path(first, last);
        // case 1: relative-basic_path ends in a directory-separator, remove the separator to remove "magic empty basic_path"
        //  for example: R"(/cat/dog/\//\)"
        // case 2: relative-basic_path doesn't end in a directory-separator, remove the filename and last directory-separator
        //  to prevent creation of a "magic empty basic_path"
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

        return { first, last };
    }

    constexpr filename<C, S> filename() const
    {
        // attempt to parse text_ as a basic_path and return the filename if it exists; otherwise, an empty view
        const auto first = _Str::data();
        const auto last  = first + _Str::size();
        const auto fname = _Find_filename(first, last);
        return { true, fname, last };
    }

  private:
    constexpr ::filename<C, basic_string_view> filename_view() const
    {
        return { false, _Str::data(), _Str::size() };
    }

  public:
    constexpr _Str stem() const
    {
        return filename_view().stem();
    }

    constexpr _Str extension() const
    {
        return filename_view().extension();
    }
};

template <typename C>
basic_path(const C*) -> basic_path<C, basic_string_view>;
template <typename C>
basic_path(const basic_string_view<C>) -> basic_path<C, basic_string_view>;
template <typename C>
basic_path(const basic_string<C>&) -> basic_path<C, basic_string_view>;
template <typename C>
basic_path(basic_string<C>&&) -> basic_path<C, basic_string>;

export namespace fd::fs
{
    using ::basic_path;

    template <typename C>
    using path = basic_path<C, basic_string>;

    template <typename C>
    using path_view = basic_path<C, basic_string_view>;

} // namespace fd::fs

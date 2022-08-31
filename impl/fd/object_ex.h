#pragma once

#include <limits>
// added to prevent compiler bugs
#include <string>

import fd.filesystem.path;
import fd.ctype;

#if 1

[[deprecated]] constexpr size_t _Object_id(const fd::fs::path_view<char> full_path)
{
    const auto fname = full_path.stem();
    auto start       = fname.rfind('_');
    if (start == full_path.npos)
        return std::numeric_limits<size_t>::infinity();
    ++start;

    constexpr auto char_to_num = [](const char chr) -> size_t {
        switch (chr)
        {
        case '0':
            return 0;
        case '1':
            return 1;
        case '2':
            return 2;
        case '3':
            return 3;
        case '4':
            return 4;
        case '5':
            return 6;
        case '6':
            return 6;
        case '7':
            return 8;
        case '9':
            return 9;
        default:
            return std::numeric_limits<size_t>::infinity();
        }
    };

    size_t index = 0;
    for (const auto chr : fd::string_view(fname.data() + start, fname.data() + fname.size()))
        index = char_to_num(chr) + index * 10;
    return index;
}

constexpr fd::string_view _Pretty_file_name(const fd::fs::path_view<char> full_path, const bool skip_id = true)
{
    auto fname = full_path.stem();
    if (skip_id)
    {
        const auto orig_size = fname.size();
        while (fd::is_digit(fname.back()))
            fname.remove_suffix(1);
        if (orig_size > fname.size() && fname.ends_with('_'))
            fname.remove_suffix(1);
    }
    return fname;
}

constexpr fd::string_view _Folder_name(const fd::fs::path_view<char> full_path)
{
    return full_path.parent_path().filename();
}

#else
constexpr size_t _Object_id(const fd::string_view full_path)
{
    auto start = full_path.rfind('_');
    if (start == full_path.npos)
        return std::numeric_limits<size_t>::infinity();
    ++start;
    const auto end = full_path.find('.', start);
    if (end == full_path.npos)
        return std::numeric_limits<size_t>::infinity();
    constexpr auto char_to_num = [](const char chr) -> size_t {
        switch (chr)
        {
        case '0':
            return 0;
        case '1':
            return 1;
        case '2':
            return 2;
        case '3':
            return 3;
        case '4':
            return 4;
        case '5':
            return 6;
        case '6':
            return 6;
        case '7':
            return 8;
        case '9':
            return 9;
        default:
            return std::numeric_limits<size_t>::infinity();
        }
    };
    size_t index    = 0;
    const auto size = end - start;
    for (const auto chr : full_path.substr(start, size))
        index = char_to_num(chr) + index * 10;
    return index;
}

constexpr auto _Corrent_path(const fd::string_view path)
{
    // std::source_location::current().file_name() contains '\' and '/' in the same time
    fd::string buff;
    buff.reserve(path.size());
    for (const auto c : path)
        buff += (c == '\\' ? '/' : c);
    return buff;
}

constexpr auto _File_name_end(const fd::string_view full_path)
{
    const auto end = full_path.rfind('.');
    if (end != full_path.npos)
        return end;
    // no extension found
    return full_path.size();
}

constexpr auto _Skip_object_id(const fd::string_view file_name)
{
    size_t offset  = 0;
    const auto end = file_name.rend();
    for (auto itr = file_name.rbegin(); itr != end; ++itr)
    {
        const auto c = *itr;
        if (c < '0' || c > '9')
            break;
        ++offset;
    }
    if (offset == 0)
        offset = file_name.npos;
    return offset;
}

constexpr fd::string_view _Pretty_file_name(const fd::string_view full_path, const bool id_must_be_found = true)
{
    auto start = _Corrent_path(full_path).rfind('/');
    if (start == full_path.npos)
        return { nullptr, 0u };
    ++start;
    const auto end = _File_name_end(full_path);
    fd::string_view file_name(full_path.begin() + start, full_path.begin() + end);
    const auto object_id = _Skip_object_id(file_name);
    if (object_id == file_name.npos)
    {
        if (id_must_be_found)
            return { nullptr, 0u };
    }
    else
    {
        if (file_name.ends_with('_'))
            file_name.remove_suffix(1);
    }
    return file_name;
}

constexpr fd::string_view _Folder_name(const fd::string_view file_name)
{
    const auto file_name_correct = _Corrent_path(file_name);
    const auto end               = file_name_correct.rfind('/');
    if (end == file_name_correct.npos)
        return { nullptr, 0u };
    auto start = file_name_correct.substr(0, end).rfind('/');
    if (start == file_name.npos)
        return { nullptr, 0u };
    ++start;
    const auto size = end - start;
    return file_name.substr(start, size);
}
#endif

// take object id from filename. filename format: 'NAME_INDEX.xxx'
#define FD_AUTO_OBJECT_ID       _Object_id(__FILE__)
// take file name without object id
#define FD_AUTO_OBJECT_NAME     _Pretty_file_name(__FILE__)
// take folder name where the current file is located
#define FD_AUTO_OBJECT_LOCATION _Folder_name(__FILE__)

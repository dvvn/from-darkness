module;

#include <fd/assert.h>

#include <filesystem>
#include <fstream>
#include <sstream>
#include <vector>

module fd.netvars.core:storage;
import fd.logger;
import fd.address;
import fd.json;

namespace std::filesystem
{
    static_assert(sizeof(path) == sizeof(std::wstring));

    bool create_directory(const std::wstring& dir)
    {
        return create_directory(reinterpret_cast<const path&>(dir));
    }

    bool is_empty(const std::wstring& dir)
    {
        return is_empty(reinterpret_cast<const path&>(dir));
    }
} // namespace std::filesystem

using namespace fd;
using namespace netvars;
namespace fs = std::filesystem;

template <typename T, typename... Args>
static T _Join_strings(const Args&... args)
{
    const auto string_size = (args.size() + ...);
    T out;
    out.reserve(string_size);
    (out.append(args.begin(), args.end()), ...);
    return out;
}

static bool _File_already_written(const fs::path& full_path, const std::string_view buffer)
{
    std::ifstream file_stored(full_path, std::ios::binary | std::ios::ate);
    if (!file_stored)
        return false;

    const auto size = static_cast<size_t>(file_stored.tellg());
    if (size != buffer.size())
        return false;

#if 0
	const auto buff = std::make_unique<char[]>(size);
	if (!file_stored.read(buff.get(), size))
		return false;
	return std::memcmp(buff.get(), buffer.data(), size) == 0;
#else
    using it_t = std::istream_iterator<char>;
    return std::equal<it_t>(file_stored, {}, buffer.begin());
#endif
}

logs_data::~logs_data()
{
    // moved
    if (dir.empty())
        return;

    if (!fs::create_directory(dir))
        return;

    const fs::path full_path = _Join_strings<std::wstring>(dir, file.name, file.extension);
    const auto new_file_data = buff.view();

    if (_File_already_written(full_path, new_file_data))
        return;

    std::ofstream(full_path) << new_file_data;
}

classes_data::~classes_data()
{
    // moved
    if (dir.empty())
        return;

    if (fs::create_directory(dir) || fs::is_empty(dir))
    {
        for (const auto& [name, buff] : files)
            std::ofstream(_Join_strings<std::wstring>(dir, name)) << buff.view();
        return;
    }

    // directory already exist

    for (const auto& [name, buff] : files)
    {
        const auto new_file_data         = buff.view();
        const fs::path current_file_path = _Join_strings<std::wstring>(dir, name);
        if (_File_already_written(current_file_path, new_file_data))
            continue;

        std::ofstream(current_file_path) << new_file_data;
    }
}

template <class J>
concept json_support_string_view = requires(J& js, const std::string_view test)
{
    js[test];
};

template <class J>
static auto& _Json_append(J& js, const std::string_view str)
{
    if constexpr (json_support_string_view<J>)
        return js[str];
    else
        return js[std::string(str)];
}

void storage::log_netvars(logs_data& data)
{
    json_unsorted j_root;

    for (const netvar_table& table : *this)
    {
        if (table.empty())
            continue;

        auto& j_table = _Json_append(j_root, table.name());

        for (const netvar_table::value_type& info : table)
        {
            const std::string_view name = info->name();
            const auto offset           = info->offset();
            const std::string_view type = info->type();

            if (type.empty())
            {
                j_table.push_back({
                    {"name",    std::string(name)},
                    { "offset", offset           }
                });
            }
            else
            {
                j_table.push_back({
                    {"name",    std::string(name)},
                    { "offset", offset           },
                    { "type",   std::string(type)}
                });
            }

            /*
            json_unsorted entry;
            _Json_append(entry, "name")   = std::string(name);
            _Json_append(entry, "offset") = offset;
            if (!type.empty())
                _Json_append(entry, "type") = std::string(type);

            j_table.push_back(std::move(entry)); */
        }
    }

    data.buff << std::setw(data.indent) << std::setfill(data.filler) << j_root;
}

void storage::generate_classes(classes_data& data)
{
    data.files.reserve(this->size());

    for (const netvar_table& table : *this)
    {
        if (table.empty())
            continue;

        classes_data::file_info h_info, cpp_info;
        auto& h   = h_info.buff;
        auto& cpp = cpp_info.buff;

        const std::string_view class_name = table.name();

        for (const netvar_table::value_type& info : table)
        {
            auto netvar_type = info->type();
            if (netvar_type.empty())
                continue;

            char ret_char;
            if (netvar_type.ends_with('*')) // type have pointer
            {
                netvar_type.remove_suffix(1);
                ret_char = '*';
            }
            else
            {
                ret_char = '&';
            }

            const std::string_view netvar_name = info->name();

            //---

            const auto write_func_header = [=](std::basic_ostream<char>& stream, const bool inside_class) {
                stream << netvar_type << ret_char << ' ';
                if (!inside_class)
                    stream << class_name << "::";
                stream << netvar_name << "( );\n";
            };

            write_func_header(h, true);

            cpp << '\n';
            write_func_header(cpp, false);
            cpp << "{\n"
                << '	' << "return netvars::apply_offset"
#ifndef FD_NETVARS_LOG_STATIC_OFFSET
                << "<\"" << class_name << "\">, <\"" << netvar_name << "\">"
#endif
                << "(this"
#ifdef FD_NETVARS_LOG_STATIC_OFFSET
                << ", " << info.offset()
#endif
                << ");\n}\n";
        }

        auto& h_name   = h_info.name;
        auto& cpp_name = cpp_info.name;

        constexpr std::wstring_view h_postfix = L"_h";
        h_name.reserve(class_name.size() + h_postfix.size());
        constexpr std::wstring_view cpp_postfix = L"_cpp";
        cpp_name.reserve(class_name.size() + cpp_postfix.size());

        h_name = cpp_name = { class_name.begin(), class_name.end() };
        h_name += h_postfix;
        cpp_name += cpp_postfix;

        data.files.push_back(std::move(h_info));
        data.files.push_back(std::move(cpp_info));
    }
}

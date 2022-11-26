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
    static_assert(sizeof(path) == sizeof(fd::wstring));

    bool create_directory(const fd::wstring& dir)
    {
        return create_directory(reinterpret_cast<const path&>(dir));
    }

    bool is_empty(const fd::wstring& dir)
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

static bool _File_already_written(const fs::path& full_path, const fd::string_view buffer)
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

void storage::log_netvars(logs_data& data)
{
}

void storage::generate_classes(classes_data& data)
{
    FD_ASSERT("REWRITE THIS SHIT");
#if 0
    data.files.reserve(this->size());

    for (const netvar_table& table : *this)
    {
        if (table.empty())
            continue;

        classes_data::file_info h_info, cpp_info;
        auto& h   = h_info.buff;
        auto& cpp = cpp_info.buff;

        const fd::string_view class_name = table.name();

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

            const fd::string_view netvar_name = info->name();

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

        constexpr fd::wstring_view h_postfix = L"_h";
        h_name.reserve(class_name.size() + h_postfix.size());
        constexpr fd::wstring_view cpp_postfix = L"_cpp";
        cpp_name.reserve(class_name.size() + cpp_postfix.size());

        h_name = cpp_name = { class_name.begin(), class_name.end() };
        h_name += h_postfix;
        cpp_name += cpp_postfix;

        data.files.push_back(std::move(h_info));
        data.files.push_back(std::move(cpp_info));
    }
#endif
}

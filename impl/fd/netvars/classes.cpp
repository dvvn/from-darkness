#include <fd/netvars/classes.h>
#include <fd/utils/file.h>

#include <boost/filesystem.hpp>
#include <boost/static_string.hpp>

#include <fmt/format.h>

#include <span>

struct chars_buffer : std::vector<char>
{
    chars_buffer() = default;

    chars_buffer(chars_buffer const& other)            = delete;
    chars_buffer& operator=(chars_buffer const& other) = delete;

    template <size_t S>
    void append_range(char const (&text)[S])
    {
        std::vector<char>::append_range(std::span(text, S - 1));
    }

#if 0
    chars_buffer& operator=(char c)
    {
        push_back(c);
        return *this;
    }

    chars_buffer& operator*()
    {
        return *this;
    }

    chars_buffer& operator++()
    {
        return *this;
    }

    using iterator_category = std::output_iterator_tag;
    using value_type        = void;
    using pointer           = void;
    using reference         = void;
#endif

    auto out()
    {
        return std::back_insert_iterator(*this);
    }
};

template <size_t S>
struct fmt::formatter<fmt::basic_memory_buffer<char, S>> : formatter<string_view>
{
    auto format(basic_memory_buffer<char, S> const& buff, format_context& ctx) const
    {
        return formatter<string_view>::format(string_view(buff.data(), buff.size()), ctx);
    }
};

namespace fd
{
struct path_info
{
    boost::filesystem::path path;
    std::span<char>         buff;

    path_info(netvars_classes const& root, netvars_classes::file_info& file)
        : path(root.dir / file.name)
        , buff(file.data)
    {
    }

    // ReSharper disable once CppMemberFunctionMayBeConst
    void write_unsafe()
    {
        write_to_file(path.c_str(), buff.data(), buff.size());
    }

    bool skip() const
    {
        return file_already_written(path.c_str(), buff.data(), buff.size());
    }

    void write()
    {
        if (!skip())
            write_unsafe();
    }
};

netvars_classes::~netvars_classes()
{
    if (dir.empty())
        return;

    if (create_directories(dir) || is_empty(dir))
    {
        for (auto& f : files)
            path_info(*this, f).write_unsafe();
    }
    else
    {
        for (auto& f : files)
            path_info(*this, f).write();
    }
}

netvars_classes::netvars_classes() = default;

struct generate_info
{
    std::string_view name;

    /*fmt::basic_memory_buffer<char,32>*/ std::string typeOut;
    /*fmt::basic_memory_buffer<char,64>*/ std::string typeCast;

    generate_info(netvar_info& info)
    {
        auto typeRaw = info.type();
        assert(!typeRaw.empty());

        auto isPointer   = typeRaw.ends_with('*');
        auto typeDecayed = isPointer ? typeRaw.substr(0, typeRaw.size() - 1) : typeRaw;

        typeOut.append(typeDecayed);
        typeOut.push_back(isPointer ? '*' : '&');

        if (isPointer)
            typeCast.push_back('*');
        typeCast.append("reinterpret_cast<");
        typeCast.append(typeDecayed);
        typeCast.append("*>");

        name = info.name();
    }
};

static constexpr auto& _FormatOff       = "// clang-format off\n"
                                          "// ReSharper disable All\n\n";
static constexpr auto& _CppIncludes     = "#include <cassert>\n"
                                          "#include <string_view>\n\n";
static constexpr auto& _NetvarsGetterFn = "size_t get_netvar_offset"
                                          "(std::string_view table, std::string_view name);\n\n";

// static void _write_header_head()

static constexpr std::string_view _HeaderSample = //
    "({FormatOff})"
    "#include <cassert>\n"
    "#include <string_view>\n\n";

static void _parse_source(chars_buffer& buff, std::string_view className, std::span<generate_info const> table)
{
    /*fmt::basic_memory_buffer<char, 32>*/ std::string offsetsClass;
    offsetsClass.append(className);
    offsetsClass.append("_offsets");

    buff.append_range(_FormatOff);
    buff.append_range(_CppIncludes);
    buff.append_range(_NetvarsGetterFn);

    buff.append_range("static struct\n{\n");
    {
        for (auto& i : table)
            fmt::format_to(buff.out(), "\tsize_t {} = -1;\n", i.name);

        buff.append_range("\n\tvoid init()\n\t{\n");
        {
            for (auto& i : table)
            {
                fmt::format_to(
                    buff.out(),
                    "\t\tassert({name} == -1, \"already set!\";\n"
                    "\t\t{name} = get_netvar_offset(\'{className}\', \'{name}\');\n",
                    fmt::arg("name", i.name),
                    fmt::arg("className", className));
            }
        }
        buff.append_range("\t}\n");
    }
    buff.push_back('}');
    fmt::format_to(buff.out(), " {};\n\n", offsetsClass);

    for (auto& i : table)
    {
        fmt::format_to(
            buff.out(),
            "{typeOut} {className}::{name}()\n"
            "{{\n"
            "\tassert({offsetsClass}.{name} != -1, \"not set!\");\n"
            "\tconst auto addr = reinterpret_cast<uintptr_t>(this) + {offsetsClass}.{name};\n"
            "\treturn {typeCast}(addr);\n"
            "}}\n",
            fmt::arg("typeOut", i.typeOut),
            fmt::arg("className", className),
            fmt::arg("name", i.name),
            fmt::arg("offsetsClass", offsetsClass),
            fmt::arg("typeCast", i.typeCast));
    }
}

static void _parse_header(chars_buffer& buff, std::span<generate_info const> table)
{
    buff.append_range(_FormatOff);
    for (auto& i : table)
    {
        fmt::format_to(buff.out(), "{} {}():\n", i.typeOut, i.name);
    }
}

void netvars_classes::fill(netvar_table& rawTable)
{
    assert(!dir.empty());

#ifdef GENERATE_STRUCT_MEMBERS
#error "not implemented"
#else

    if (rawTable.empty())
        return;

    auto filesCount = files.size();
    files.resize(filesCount + rawTable.size() * 2);
    auto file = filesCount + files.begin();

    std::vector<generate_info> table;
    table.assign_range(rawTable);

    auto className = rawTable.name();

    chars_buffer source;
    _parse_source(source, className, table);
    chars_buffer header;
    _parse_header(header, table);

    auto storeFile = [&](std::string_view extension, std::vector<char>& buff)
    {
        assert(file->name.empty());
        file->name.append_range(className).append_range(extension);
        file->data = std::move(buff);
        ++file;
    };
    storeFile("_h", header);
    storeFile("_cpp", source);

    // todo: if !FD_NETVARS_DT_MERGE include wanted files!

#endif
}

} // namespace fd
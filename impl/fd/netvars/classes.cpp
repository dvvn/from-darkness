#include <fd/netvars/classes.h>
#include <fd/utils/file.h>

#include <boost/filesystem.hpp>

#include <fmt/format.h>

#include <algorithm>
#include <span>

// template <size_t S>
// struct fmt::formatter<fmt::basic_memory_buffer<char, S>> : formatter<string_view>
//{
//     auto format(basic_memory_buffer<char, S> const &buff, format_context &ctx) const
//     {
//         return formatter<string_view>::format(string_view(buff.data(), buff.size()), ctx);
//     }
// };

namespace fd
{
netvar_classes::~netvar_classes()
{
    if (dir.empty())
        return;

    if (create_directories(dir) || is_empty(dir))
    {
        for (auto &f : files_)
            write_file((dir / f.name).native(), f.data);
    }
    else
    {
        for (auto &f : files_)
            write_file((dir / f.name).native(), f.data, false);
    }
}

netvar_classes::netvar_classes() = default;

struct generate_info
{
    netvar_type_merged_includes<std::string_view> include;
    std::string_view name;
    std::string type_out;
    std::string type_cast;

    generate_info(netvar_info &info)
    {
        auto type_raw = info.type();
        assert(!type_raw.empty());

        auto is_pointer   = type_raw.ends_with('*');
        auto type_decayed = is_pointer ? type_raw.substr(0, type_raw.size() - 1) : type_raw;

        type_out.append(type_decayed);
        type_out.push_back(is_pointer ? '*' : '&');

        if (is_pointer)
            type_cast.push_back('*');
        type_cast.append("reinterpret_cast<").append(type_decayed).append("*>");

        name = info.name();

        info.type_ex().write_includes(std::back_inserter(include));
        /*std::stable_sort(include.begin(), include.end());
        include.erase(std::unique(include.begin(), include.end()), include.end());*/
    }
};

struct chars_buffer : std::vector<char>
{
    chars_buffer() = default;

    chars_buffer(chars_buffer const &other)            = delete;
    chars_buffer &operator=(chars_buffer const &other) = delete;

    using std::vector<char>::append_range;

    template <size_t S>
    void append_range(char const (&text)[S])
    {
        append_range(std::initializer_list<char>(std::begin(text), std::end(text) - 1));
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

class filler
{
    static constexpr auto &format_off_ = "// clang-format off\n"
                                         "// ReSharper disable All\n\n";

    static constexpr auto &cpp_includes_      = "#include <cassert>\n"
                                                "#include <string_view>\n\n";
    static constexpr auto &netvars_getter_fn_ = "size_t get_netvar_offset"
                                                "(std::string_view table, std::string_view name);\n\n";

    std::vector<generate_info> table_;
    std::string_view class_name_;

    chars_buffer buff_;
    std::vector<file_info> *files_;

    void store(std::string_view extension) const
    {
        auto name = std::wstring().append_range(class_name_).append_range(extension);
        files_->emplace_back(std::move(name), std::move(buff_));
    }

  public:
    filler(netvar_table &raw_table, std::vector<file_info> &files)
        : table_(raw_table.begin(), raw_table.end())
        , class_name_(raw_table.name())
        , files_(&files)
    {
    }

    void source(std::string_view extension = "_cpp")
    {
        std::string offsets_class;
        offsets_class.append(class_name_).append("_offsets");

        buff_.append_range(format_off_);
        buff_.append_range(cpp_includes_);
        buff_.append_range(netvars_getter_fn_);

        buff_.append_range("static struct\n{\n");
        {
            for (auto &i : table_)
                fmt::format_to(buff_.out(), "\tsize_t {} = -1;\n", i.name);

            buff_.append_range("\n\tvoid init()\n\t{\n");
            {
                for (auto &i : table_)
                {
                    fmt::format_to(
                        buff_.out(),
                        "\t\tassert({name} == -1, \"already set!\";\n"
                        "\t\t{name} = get_netvar_offset(\'{class_name}\', \'{name}\');\n",
                        fmt::arg("name", i.name),
                        fmt::arg("class_name", class_name_));
                }
            }
            buff_.append_range("\t}\n");
        }
        buff_.push_back('}');
        fmt::format_to(buff_.out(), " {}_offsets;\n\n", class_name_);

        for (auto &i : table_)
        {
            fmt::format_to(
                buff_.out(),
                "{typeOut} {class_name}::{name}()\n"
                "{{\n"
                "\tassert({offsetsClass}.{name} != -1, \"not set!\");\n"
                "\tconst auto addr = reinterpret_cast<uintptr_t>(this) + {offsetsClass}.{name};\n"
                "\treturn {typeCast}(addr);\n"
                "}}\n",
                fmt::arg("typeOut", i.type_out),
                fmt::arg("class_name", class_name_),
                fmt::arg("name", i.name),
                fmt::arg("offsetsClass", offsets_class),
                fmt::arg("typeCast", i.type_cast));
        }

        store(extension);
    }

    void header(std::string_view extension = "_h")
    {
        buff_.append_range(format_off_);
        for (auto &i : table_)
        {
            fmt::format_to(buff_.out(), "{} {}();\n", i.type_out, i.name);
        }

        store(extension);
    }

    void includes(std::string_view extension = "_inc")
    {
        std::vector<std::string_view> tmp;
        for (auto &info : table_)
            tmp.append_range(info.include);

        auto start = tmp.begin();
        auto end   = tmp.end();

        std::stable_sort(start, end, std::greater());
        end = std::unique(start, end);

        if (start == end)
            return;

        do
        {
            buff_.append_range(*start);
            buff_.emplace_back('\n');
        }
        while (++start != end);

        store(extension);
    }
};

void netvar_classes::fill(netvar_table &table)
{
    assert(!dir.empty());

#ifdef GENERATE_STRUCT_MEMBERS
#error "not implemented"
#else
    if (table.empty())
        return;

    auto f = filler(table, files_);

    f.source();
    f.header();
    f.includes();
#endif
}

} // namespace fd
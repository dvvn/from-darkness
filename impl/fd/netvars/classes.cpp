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

    generate_info(netvar_info &info)
        : name(info.name())
        , type_out(info.type())
    {
        assert(!type_out.empty());

        if (!type_out.ends_with('*'))
            type_out.push_back('&');

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

    std::vector<generate_info> table_;
    std::string_view class_name_;

    chars_buffer buff_;
    std::vector<file_info> *files_;

    void store(std::string_view extension)
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
        buff_.append_range(format_off_);
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
                        "\t\tset_netvar_offset(this->{}, \"{}\", \"{}\");\n", //
                        i.name,
                        class_name_,
                        i.name);
                }
            }
            buff_.append_range("\t}\n");
        }
        buff_.append_range("} current_class_netvars;\n\n");

        for (auto &i : table_)
        {
            fmt::format_to(
                buff_.out(),
                "{typeOut} NETVAR_CLASS::{name}()\n"
                "{{\n"
                "\treturn get_netvar<{typeOut}>(this, current_class_netvars.{name});\n"
                "}}\n",
                fmt::arg("typeOut", i.type_out),
                fmt::arg("name", i.name));
        }

        buff_.append_range("static void init_current_netvars()\n"
                           "{\n"
                           "\tcurrent_class_netvars.init();\n"
                           "}\n");

        store(extension);
    }

    void s_includes(std::string_view extension = "_cpp_inc")
    {
#ifdef FD_WORK_DIR
        using boost::filesystem::path;
        assert(exists(path(BOOST_STRINGIZE(FD_WORK_DIR)).append("netvars/getter.h")));
#endif
        buff_.append_range(format_off_);
        buff_.append_range("#include <fd/netvars/getter.h>\n");
        // buff_.append_range("#include <cassert>\n\n");

        /*buff_.append_range("size_t get_netvar_offset"
                           "(std::string_view table, std::string_view name);\n\n");*/

        store(extension);
    }

    void header(std::string_view extension = "_h")
    {
        buff_.append_range(format_off_);
        for (auto &i : table_)
            fmt::format_to(buff_.out(), "{} {}();\n", i.type_out, i.name);

        store(extension);
    }

    void h_includes(std::string_view extension = "_h_inc")
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

        buff_.append_range(format_off_);

        do
            fmt::format_to(buff_.out(), "#include {}\n", *start);
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
    f.s_includes();
    f.header();
    f.h_includes();
#endif
}

} // namespace fd
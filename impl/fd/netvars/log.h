#pragma once

#include <fd/netvars/basic_table.h>

#include <boost/filesystem/path.hpp>

#include <string>
#include <vector>

namespace fd
{
struct netvars_log final
{
    using fill_fn = std::function<bool(basic_netvar_table const*&)>;

    ~netvars_log();
    netvars_log();

    boost::filesystem::path dir;

    struct
    {
        std::wstring      name;
        std::wstring_view extension;
    } file;

    uint8_t indent;
    char    filler;

    boost::filesystem::path make_path() const;

    size_t fill(fill_fn const& updater);

  private:
    std::vector<char> buff;
};
} // namespace fd
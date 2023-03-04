#pragma once

#include <fd/netvars/table.h>

#include <boost/filesystem/path.hpp>

#include <string>
#include <vector>

namespace fd
{
class netvars_log final
{
    std::vector<char> buff_;

  public:
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

    void fill(netvar_table& table);
};
} // namespace fd
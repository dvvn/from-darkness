#pragma once

#include "../table.h"

#include <boost/filesystem/path.hpp>

namespace fd
{
struct file_info
{
    std::wstring name;
    std::vector<char> data;
};

class netvar_classes final
{
    std::vector<file_info> files_;

  public:
    ~netvar_classes();
    netvar_classes();

    boost::filesystem::path dir;

    void fill(netvar_table &table);
};
} // namespace fd
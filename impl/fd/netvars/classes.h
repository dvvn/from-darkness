#pragma once

#include <fd/netvars/table.h>

#include <boost/filesystem/path.hpp>

namespace fd
{
class netvars_classes final
{
    friend struct path_info;
    
    struct file_info
    {
        std::wstring      name;
        std::vector<char> data;
    };

    std::vector<file_info> files;

  public:
    ~netvars_classes();
    netvars_classes();

    boost::filesystem::path dir;

    void fill(netvar_table & table);
};
} // namespace fd
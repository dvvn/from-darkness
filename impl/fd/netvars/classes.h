#pragma once

#include <fd/netvars/basic_table.h>

#include <boost/filesystem/path.hpp>

namespace fd
{
struct netvars_classes final
{
        using fill_fn = std::function<bool(basic_netvar_table const*&)>;

    
    ~netvars_classes();
    netvars_classes();

    boost::filesystem::path dir;

    struct file_info
    {
        std::wstring      name;
        std::vector<char> data;
    };

    bool fill(fill_fn const& updater, size_t dataSize);

  private:
    std::vector<file_info> files;
};
} // namespace fd
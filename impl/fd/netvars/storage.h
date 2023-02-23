#pragma once

#include <fd/netvars/basic_storage.h>
#include <fd/netvars/table.h>
#include <fd/valve/client_class.h>
#include <fd/valve/data_map.h>

#include <vector>

namespace fd
{
struct netvars_log final
{
    ~netvars_log();
    netvars_log();

    std::wstring dir;

    struct
    {
        std::wstring name;
        std::wstring extension;
    } file;

    uint8_t indent;
    char    filler;

    std::vector<char> buff;
};

struct netvars_classes final
{
    ~netvars_classes();
    netvars_classes();

    std::wstring dir;

    struct file_info
    {
        std::wstring      name;
        std::vector<char> data;
    };

    std::vector<file_info> files;
};

class netvars_storage final : public basic_netvars_storage
{
    std::vector<netvar_table> data_;
    std::vector<size_t>       sortRequested_;

  public:
    void request_sort(netvar_table const* table);

  private:
    void sort();

    void add(netvar_table&& table);

  public:
    netvar_table* add(std::string&& name, bool root);
    netvar_table* add(std::string_view name, bool root);

    netvars_storage();

    netvar_table*       find(std::string_view name);
    netvar_table const* find(std::string_view name) const;

    void iterate_client_class(valve::client_class const* rootClass);
    void iterate_datamap(valve::data_map const* rootMap);

    void log_netvars(netvars_log& data);
    void generate_classes(netvars_classes& data);

    void   finish();
    size_t get_offset(std::string_view className, std::string_view name) const override;
    void   clear();
};
} // namespace fd
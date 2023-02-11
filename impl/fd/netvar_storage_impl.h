#pragma once

#include <fd/netvar_storage.h>
#include <fd/netvar_table.h>
#include <fd/valve/client_class.h>
#include <fd/valve/data_map.h>

#include <vector>

namespace fd
{
struct netvars_log
{
    ~netvars_log();
    netvars_log();

    wstring dir;

    struct
    {
        wstring name;
        wstring extension;
    } file;

    uint8_t indent;
    char    filler;

    std::vector<char> buff;
};

struct netvars_classes
{
    ~netvars_classes();
    netvars_classes();

    wstring dir;

    struct file_info
    {
        wstring           name;
        std::vector<char> data;
    };

    std::vector<file_info> files;
};

class netvars_storage final : public basic_netvars_storage
{
    std::vector<netvar_table> data_;
    std::vector<size_t>       sortRequested_;

  public:
    void request_sort(const netvar_table* table);

  private:
    void sort();

  public:
    netvar_table* add(string&& name, bool root = true);
    netvar_table* add(string_view name, bool root = true);

    netvars_storage();

    netvar_table*       find(string_view name);
    const netvar_table* find(string_view name) const;

    void iterate_client_class(const valve::client_class* rootClass, string_view debugName = {});
    void iterate_datamap(const valve::data_map* rootMap, string_view debugName = {});
    void log_netvars(netvars_log& data);
    void generate_classes(netvars_classes& data);

    void   finish();
    size_t get_offset(string_view className, string_view name) const override;
    void   clear();
};
} // namespace fd
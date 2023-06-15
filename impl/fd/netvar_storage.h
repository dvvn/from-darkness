#pragma once

#include "basic_netvar_storage.h"
#include "tool/string.h"
#include "tool/string_view.h"
#include "tool/variant.h"
#include "tool/vector.h"

namespace fd
{
namespace valve
{
struct client_class;
struct recv_prop;
struct recv_table;
struct data_map_description;
struct data_map;
} // namespace valve

struct basic_netvar_type_cache
{
    virtual string_view get(void *key) const             = 0;
    virtual string_view store(void *key, string &&value) = 0;
};

using netvar_source = variant<valve::recv_prop *, valve::data_map_description *>;

using netvar_type_stored = variant<string, basic_netvar_type_cache *>;

class netvar_info : public basic_netvar_info
{
    netvar_source source_;
    netvar_type_stored type_;

  public:
    netvar_info(netvar_source source, basic_netvar_type_cache *type_cache /*= nullptr*/);

    string_view raw_name() const;
    string_view name() const;
    string_view type() const;
    size_t offset() const override;
};

using netvar_table_source = variant<valve::recv_table *, valve::data_map *>;

class netvar_table : public basic_netvar_table
{
    netvar_table_source source_;
    vector<netvar_info> storage_;

  public:
    netvar_table(netvar_table_source source, basic_netvar_type_cache *type_cache);

    basic_netvar_table *inner(string_view name) override;
    basic_netvar_info *get(string_view name) override;

    string_view raw_name() const;
    string_view name() const;

    bool empty() const;
};

class netvar_type_cache : public basic_netvar_type_cache
{
    struct stored_value
    {
        void *key;
        string type;

        string_view view() const
        {
            return {type.begin(), type.end()};
        }
    };

    vector<stored_value> storage_;

  public:
    string_view get(void *key) const override;
    string_view store(void *key, string &&value) override;
};

class netvar_storage : public basic_netvar_storage
{
    netvar_type_cache class_types_;
    netvar_type_cache datamap_types_;
    vector<netvar_table> storage_;

  public:
    basic_netvar_table *get(string_view name) override;

    void store(valve::client_class *root);
    void store(valve::data_map *root);
};
}
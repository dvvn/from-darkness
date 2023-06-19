#pragma once

#include "basic_netvar_storage.h"
#include "tool/functional.h"
#include "tool/string.h"
#include "tool/string_view.h"
#include "tool/variant.h"
#include "tool/vector.h"

#define FD_MERGE_NETVAR_TABLES

namespace fd
{
namespace valve
{
struct client_class;
struct recv_prop;
struct recv_table;
struct data_map_field;
struct data_map;
} // namespace valve

struct basic_netvar_type_cache
{
    virtual string_view get(void *key) const             = 0;
    virtual string_view store(void *key, string &&value) = 0;
};

using netvar_source      = variant<valve::recv_prop *, valve::data_map_field *>;
using netvar_type_stored = variant<string, basic_netvar_type_cache *>;

class netvar_info : public basic_netvar_info
{
    netvar_source source_;
    netvar_type_stored type_;
#ifdef FD_MERGE_NETVAR_TABLES
    size_t extra_offset_;
#endif

  public:
#ifdef FD_MERGE_NETVAR_TABLES
    netvar_info(netvar_source source, size_t extra_offset, basic_netvar_type_cache *type_cache);
    netvar_info(netvar_source source, size_t extra_offset);
#else
    netvar_info(netvar_source source, basic_netvar_type_cache *type_cache);
    netvar_info(netvar_source source);
#endif

    char const *raw_name() const;
    size_t raw_offset() const;

    char const *name() const;
    size_t offset() const override;
    string_view type() const;
};

using netvar_table_source = variant<valve::recv_table *, valve::data_map *>;

class netvar_table : public basic_netvar_table
{
    friend class netvar_storage;

    netvar_table_source source_;
    vector<netvar_info> storage_;
#ifndef FD_MERGE_NETVAR_TABLES
    vector<netvar_table> inner_;
#endif

    void parse_recv_table(
        valve::recv_table *recv,
        size_t offset,
        basic_netvar_type_cache *type_cache,
        bool filter_duplicates);
    void parse_data_map(
        valve::data_map *dmap,
        size_t offset,
        basic_netvar_type_cache *type_cache,
        bool filter_duplicates);

  public:
    netvar_table(netvar_table_source source, basic_netvar_type_cache *type_cache);

    basic_netvar_table *inner(string_view name) override;
    basic_netvar_info *get(string_view name) override;
    basic_netvar_info *get_raw(char const *name);

    char const *raw_name() const;
    char const *name() const;

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
    struct data_storage
    {
        netvar_type_cache types;
        vector<netvar_table> data;
    };

    data_storage data_tables_;
    data_storage data_maps_;

  public:
    basic_netvar_table *get(string_view name) override;

    void store(valve::client_class *root);
    void store(valve::data_map *root);
};
}
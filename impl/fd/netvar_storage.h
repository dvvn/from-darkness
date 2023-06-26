#pragma once

#include "basic_netvar_storage.h"

#include "tool/string.h"
#include "tool/variant.h"
#include "tool/vector.h"
#include "valve/data_map.h"
#include "valve/recv_table.h"

#define FD_MERGE_NETVAR_TABLES

namespace fd
{
struct native_client_class;

template <class>
struct span;

struct basic_netvar_type_cache
{
    using key_type = void const *;

    virtual string_view get(key_type key) const             = 0;
    virtual string_view store(key_type key, string &&value) = 0;
};

using netvar_source      = variant<native_recv_table::prop *, native_data_map::field *>;
using netvar_type_stored = variant<string, basic_netvar_type_cache *>;

class netvar_info : public basic_netvar_info
{
    // friend class netvar_table;

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
    /**
     * \brief dont compare internal names with it!!!
     */
    string_view pretty_name() const;
    size_t offset() const override;
    string_view type() const;
};

using netvar_table_source = variant<native_recv_table *, native_data_map *>;

class netvar_table : public basic_netvar_table
{
    friend class netvar_storage;

    using view_type = span<netvar_info const>;

    netvar_table_source source_;
    vector<netvar_info> storage_;
#ifndef FD_MERGE_NETVAR_TABLES
    vector<netvar_table> inner_;
#endif

    void parse_recv_table(
        native_recv_table *recv,
        size_t offset,
        basic_netvar_type_cache *type_cache,
        bool filter_duplicates);
    void parse_data_map(
        native_data_map *dmap,
        size_t offset,
        basic_netvar_type_cache *type_cache,
        bool filter_duplicates);

    /**
     * \brief for internal use only
     * \param name pointer to other netvar name stored in game's memory
     */
    basic_netvar_info *get_raw(char const *name);

  public:
    netvar_table(netvar_table_source source, basic_netvar_type_cache *type_cache);

    basic_netvar_table *inner(string_view name) override;
    basic_netvar_info *get(string_view name) override;

    view_type view() const;

    char const *raw_name() const;
    char const *name() const;

    bool empty() const;
};

class netvar_type_cache : public basic_netvar_type_cache
{
    struct stored_value
    {
        key_type key;
        string type;

        string_view view() const
        {
            return {type.begin(), type.end()};
        }
    };

    vector<stored_value> storage_;

  public:
    string_view get(key_type key) const override;
    string_view store(key_type key, string &&value) override;
};

class netvar_storage : public basic_netvar_storage
{
    struct data_storage : unwrap_t<vector<netvar_table>>
    {
        netvar_type_cache types;
    };

    data_storage recv_tables_;
    data_storage data_maps_;

  public:
    basic_netvar_table *get(string_view name) override;

    void store(native_client_class *root);
    void store(native_data_map *root);
};
}
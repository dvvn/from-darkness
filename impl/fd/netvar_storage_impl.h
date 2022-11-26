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

        size_t indent;
        char filler;

        std::vector<char> buff;
    };

    struct netvars_classes
    {
        ~netvars_classes();
        netvars_classes();

        wstring dir;

        struct file_info
        {
            wstring name;
            std::vector<char> data;
        };

        std::vector<file_info> files;
    };

    class netvars_storage : public basic_netvars_storage
    {
        std::vector<netvar_table> data_;
        std::vector<size_t> sort_requested_;
        // std::once_flag_ init_flag_;

        void request_sort(const netvar_table* table);
        void sort();

        netvar_table* add(string&& name, const bool root = true);
        netvar_table* add(const string_view name, const bool root = true);

      public:
        netvars_storage();

        netvar_table* find(const string_view name);
        const netvar_table* find(const string_view name) const;

        void iterate_client_class(const valve::client_class* root_class, const string_view debug_name = {});
        void iterate_datamap(const valve::data_map* root_map, const string_view debug_name = {});
        [[deprecated("Do it manually")]] void store_handmade_netvars();
        void log_netvars(netvars_log& data);
        void generate_classes(netvars_classes& data);

        void finish();
        size_t get_offset(const string_view class_name, const string_view name) const override;
    };
} // namespace fd

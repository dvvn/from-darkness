module;

#include <fd/object.h>

#include <vector>

export module fd.netvars.storage;
export import fd.netvars.basic_storage;
import fd.valve.client_class;
import fd.valve.data_map;

export namespace fd
{
    struct logs_data
    {
        ~logs_data();
        logs_data();

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

    struct classes_data
    {
        ~classes_data();
        classes_data();

        wstring dir;

        using file_info = std::pair<wstring, std::vector<char>>; // file_name, file_data_buff
        std::vector<file_info> files;
    };

    struct netvars_storage : basic_netvars_storage
    {
        virtual void iterate_client_class(const valve::client_class* root_class, const string_view debug_name = "") = 0;
        virtual void iterate_datamap(const valve::data_map* root_map, const string_view debug_name = "")            = 0;
        virtual void store_handmade_netvars()                                                                       = 0;

        virtual void log_netvars(logs_data& data)         = 0;
        virtual void generate_classes(classes_data& data) = 0;
    };
} // namespace fd

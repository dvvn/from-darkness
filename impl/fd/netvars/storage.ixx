module;

#include <fd/core/utility.h>

#include <sstream>
#include <string>
#include <vector>

export module fd.netvars.core:storage;
import :basic_storage;
import fd.csgo.structs.ClientClass;

export namespace fd::netvars
{
    struct logs_data
    {
        ~logs_data();

        std::wstring dir = FD_CONCAT(L"", FD_STRINGIZE(FD_ROOT_DIR), "/.dumps/netvars");

        struct
        {
            std::wstring name;
            std::wstring extension = L".json";
        } file;

        size_t indent = 4;
        char filler   = ' ';

        std::ostringstream buff;
    };

    struct classes_data
    {
        ~classes_data();

        std::wstring dir = FD_CONCAT(L"", FD_STRINGIZE(FD_WORK_DIR), "/external_interfaces");

        struct file_info
        {
            std::wstring name;
            std::ostringstream buff;
        };

        std::vector<file_info> files;
    };

    struct storage : basic_storage
    {
        void iterate_client_class(csgo::ClientClass* const root_class);
        void iterate_datamap(csgo::datamap_t* const root_map);
        void store_handmade_netvars();

        void log_netvars(logs_data& data);
        void generate_classes(classes_data& data);
    };
} // namespace fd::netvars

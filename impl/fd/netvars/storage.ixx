module;

#include <fd/utility.h>

#include <sstream>
#include <vector>

export module fd.netvars.core:storage;
import :basic_storage;
import fd.valve.client_class;

using namespace fd::valve;

export namespace fd::netvars
{
    struct logs_data
    {
        ~logs_data();

        wstring dir = FD_CONCAT(L"", FD_STRINGIZE(FD_ROOT_DIR), "/.dumps/netvars");

        struct
        {
            wstring name;
            wstring extension = L".json";
        } file;

        size_t indent = 4;
        char filler   = ' ';

        std::ostringstream buff;
    };

    struct classes_data
    {
        ~classes_data();

        wstring dir = FD_CONCAT(L"", FD_STRINGIZE(FD_WORK_DIR), "/external_interfaces");

        struct file_info
        {
            wstring name;
            std::ostringstream buff;
        };

        std::vector<file_info> files;
    };

    struct storage : basic_storage
    {
        void iterate_client_class(client_class* const root_class);
        void iterate_datamap(data_map* const root_map);
        void store_handmade_netvars();

        void log_netvars(logs_data& data);
        void generate_classes(classes_data& data);
    };
} // namespace fd::netvars

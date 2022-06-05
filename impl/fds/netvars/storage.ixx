﻿module;

#include <fds/core/utility.h>

#include <sstream>
#include <string>
#include <vector>

export module fds.netvars.core:storage;
import :basic_storage;
import fds.csgo.structs.ClientClass;

export namespace fds::netvars
{
    struct logs_data
    {
        ~logs_data();

        std::wstring dir = FDS_STRINGIZE_RAW_WIDE(FDS_CONCAT(VS_SolutionDir, \.dumps\netvars\));

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

        std::wstring dir = FDS_STRINGIZE_RAW_WIDE(FDS_CONCAT(VS_SolutionDir, \impl\fds\csgo\interfaces_custom\));

        struct file_info
        {
            std::wstring name;
            std::ostringstream buff;
        };

        std::vector<file_info> files;
    };

    class storage : public basic_storage
    {
      public:
        void iterate_client_class(csgo::ClientClass* const root_class);
        void iterate_datamap(csgo::datamap_t* const root_map);
        void store_handmade_netvars();

        void log_netvars(logs_data& data);
        void generate_classes(classes_data& data);
    };
} // namespace fds::netvars

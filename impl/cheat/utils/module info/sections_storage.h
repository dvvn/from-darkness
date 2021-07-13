#pragma once

#include "data_cache.h"
#include "cheat/utils/memory block.h"

namespace cheat::utl::detail
{
    struct section_info
    {
        memory_block block;
        IMAGE_SECTION_HEADER* data = nullptr;
    };
    class sections_storage final: public data_cache_from_memory<section_info>
    {
    public:
        sections_storage(address addr = 0u, IMAGE_NT_HEADERS* nt = nullptr);

    protected:
        module_info_rw_result Load_from_memory_impl( ) override;
        void              Change_base_address_impl(address new_addr) override;
    };
}

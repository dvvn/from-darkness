#pragma once

#include "data_cache.h"

namespace cheat::utl::detail
{
    class exports_storage final: public data_cache_from_anywhere<address>
    {
    public:
        exports_storage(address addr = static_cast<size_t>(0), IMAGE_NT_HEADERS* nt = nullptr);

    protected:
        module_info_rw_result Load_from_memory_impl( ) override;
        module_info_rw_result Write_to_file_impl(ptree_type& cache) const override;
        module_info_rw_result Load_from_file_impl(const ptree_type& cache) override;
        void              Change_base_address_impl(address new_addr) override;
    };
};

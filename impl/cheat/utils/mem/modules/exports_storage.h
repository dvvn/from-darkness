#pragma once

#include "data_cache.h"

namespace cheat::utl::mem
{
    class exports_storage final: public detail::data_cache_from_anywhere<address>
    {
    public:
        exports_storage(address addr = static_cast<size_t>(0), IMAGE_NT_HEADERS* nt = nullptr);

    protected:
        data_cache_result Load_from_memory_impl( ) override;
        data_cache_result Write_to_file_impl(detail::ptree_type& cache) const override;
        data_cache_result Load_from_file_impl(const detail::ptree_type& cache) override;
        void              Change_base_address_impl(address new_addr) override;
    };
};

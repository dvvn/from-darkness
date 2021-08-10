#pragma once

#include "data_cache.h"
#include "cheat/utils/memory block.h"

namespace cheat::utl::detail
{
	struct section_info
	{
		memory_block          block;
		IMAGE_SECTION_HEADER* data = nullptr;
	};

	class sections_storage: public module_data_mgr<section_info>
	{
	protected:
		bool load_from_memory(cache_type& cache) override;
		bool load_from_file(cache_type& cache, const ptree_type& storage) override;
		bool read_to_storage(const cache_type& cache, ptree_type& storage) const override;
	};

	template <size_t Offset>
	class sections_storage_ex: public sections_storage
	{
	protected:
		module_info* root_class( ) const final
		{
			return address(this).remove(Offset).ptr<module_info>( );
		}
	};
}

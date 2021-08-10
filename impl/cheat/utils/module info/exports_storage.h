#pragma once

#include "data_cache.h"

namespace cheat::utl::detail
{
	class exports_storage : public module_data_mgr<address>
	{
	protected:
		bool load_from_memory(cache_type& cache) override;
		bool load_from_file(cache_type& cache, const ptree_type& storage) override;
		bool read_to_storage(const cache_type& cache, ptree_type& storage) const override;
	};


	template<size_t Offset>
	class exports_storage_ex:public exports_storage
	{
	protected:
		module_info* root_class( ) const final
		{
			return address(this).remove(Offset).ptr<module_info>( );
		}
	};

};

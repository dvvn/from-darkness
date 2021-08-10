#pragma once

#include "data_cache.h"
#include "sections_storage.h"

#include "cheat/utils/one_instance.h"

namespace cheat::utl::detail
{
	//class memory_block;
	//using vtable_info = memory_block;

	struct vtable_info
	{
		address addr;
	};

	class vtables_storage: public module_data_mgr<vtable_info>
	{
	public:
		vtables_storage( );

		void lock( );
		void unlock( );

	protected:
		bool load_from_memory(cache_type& cache) override;
		bool load_from_file(cache_type& cache, const ptree_type& storage) override;
		bool read_to_storage(const cache_type& cache, ptree_type& storage) const override;

	private:
		sections_storage& derived_sections( ) const;
		memory_block      derived_mem_block( ) const;

		shared_ptr<mutex> lock__;
	};

	template <size_t Offset>
	class vtables_storage_ex: public vtables_storage
	{
	protected:
		module_info* root_class( ) const final
		{
			return address(this).remove(Offset).ptr<module_info>( );
		}
	};

	//since cache added this is uselles
	/*class vtables_storage_prefixed final: public detail::_Storage_for<vtables_storage>
	{
	public:
	};*/
}

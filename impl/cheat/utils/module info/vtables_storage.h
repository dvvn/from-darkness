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

	class vtables_storage final: public data_cache_from_anywhere<vtable_info>
	{
	public:
		vtables_storage(address addr = 0u, size_t bytes_count = 0, IMAGE_NT_HEADERS* nt = nullptr, sections_storage* sections = 0);

		void set_sections(sections_storage* sections);

	protected:
		module_info_rw_result Load_from_memory_impl( ) override;

		module_info_rw_result Write_to_file_impl(property_tree::ptree& cache) const override;
		module_info_rw_result Load_from_file_impl(const property_tree::ptree& cache) override;
		void Change_base_address_impl(address new_addr) override;

		memory_block Mem_block( ) const;

	private:
		size_t bytes_count__;
		sections_storage* sections__;
	};

	//since cache added this is uselles
	/*class vtables_storage_prefixed final: public detail::_Storage_for<vtables_storage>
	{
	public:
	};*/
}

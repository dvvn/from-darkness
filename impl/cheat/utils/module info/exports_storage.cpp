#include "exports_storage.h"
#include "module info.h"

using namespace cheat;
using namespace utl;
using namespace utl::detail;
using namespace property_tree;

exports_storage::exports_storage(address addr, IMAGE_NT_HEADERS* nt): data_cache_from_anywhere(move(addr), nt)
{
}

module_info_rw_result exports_storage::Load_from_memory_impl( )
{
	// get export data directory.
	const auto data_dir = &nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
	if (!data_dir->VirtualAddress)
		return error;

	// get export dir.
	const auto dir = base_address.add(data_dir->VirtualAddress).ptr<IMAGE_EXPORT_DIRECTORY>( );
#ifdef NDEBUG
    if (!dir)
        return module_info_rw_result::error;
#endif
	// names / funcs / ordinals ( all of these are RVAs ).
	const auto names = base_address.add(dir->AddressOfNames).ptr<uint32_t>( );
	const auto funcs = base_address.add(dir->AddressOfFunctions).ptr<uint32_t>( );
	const auto ords = base_address.add(dir->AddressOfNameOrdinals).ptr<uint16_t>( );
#ifdef NDEBUG
    if (!names || !funcs || !ords)
        return module_info_rw_result::error;
#endif

	const auto all_modules = all_modules::get_ptr( );
	all_modules->update(false);

	cache_type temp_cache;

	// iterate names array.
	for (auto i = 0u; i < dir->NumberOfNames; ++i)
	{
		const string_view export_name = base_address.add(names[i]).ptr<const char>( );
		if (export_name.empty( ) /*|| export_name.starts_with('?') || export_name.starts_with('@')*/)
			continue;

		/*
		 if (export_addr.cast<uintptr_t>() >= reinterpret_cast<uintptr_t>(export_directory)
			&& export_addr.cast<uintptr_t>() < reinterpret_cast<uintptr_t>(export_directory) + data_directory->Size)
		 */

		//if (export_ptr < dir || export_ptr >= memory_block(dir, data_dir->Size).addr( ))
		if (const auto export_ptr = base_address + funcs[ords[i]]; export_ptr < dir || export_ptr >= address(dir) + data_dir->Size)
		{
			temp_cache.emplace(export_name, export_ptr);
		}
		else // it's a forwarded export, we must resolve it.
		{
			// get forwarder string.
			const string_view fwd_str = export_ptr.ptr<const char>( );

			// forwarders have a period as the delimiter.
			const auto delim = fwd_str.find_last_of('.');
			if (delim == fwd_str.npos)
				continue;

			// get forwarder mod name.
			const auto fwd_module = fwd_str.substr(0, delim);
			const auto fwd_module_lower = ranges::views::transform(fwd_module, tolower);
			//const auto fwd_module_hash  = hashed_string_tag::_Compute_hash(fwd_module_lower, (".dll"));
			const auto fwd_module_str = string(fwd_module_lower.begin( ), fwd_module_lower.end( )).append(".dll");
			//const auto fwd_module_hash = module_info::create_hash(fwd_module_str);

			// get forwarder export name.
			const auto fwd_export = fwd_str.substr(delim + 1);

			// get real export ptr ( recursively ).
			//const auto target = _RANGES find(all_modules, fwd_module_hash, &module_info::name);

			auto target = all_modules->find(fwd_module_str);
			if (!target)
				continue;

			static_assert(!std::is_const_v<decltype(target)>, "unable to preload cache!");

			auto& exports = target->exports( );
			exports.load_from_memory( );
			const auto& exports_cache = exports.get_cache( );
			const auto fwd_export_ptr = exports_cache.find((fwd_export));
			if (fwd_export_ptr == exports_cache.end( ))
				continue;

			temp_cache.emplace(export_name, fwd_export_ptr->second);
		}
	}

	data_cache = move(temp_cache);
	//data_cache_.shrink_to_fit( );
	return success;
}

module_info_rw_result exports_storage::Write_to_file_impl(ptree_type& cache) const
{
	for (const auto& [name, addr]: data_cache)
	{
		cache.put(name, addr.remove(base_address).value( ));
	}

	return success;
}

module_info_rw_result exports_storage::Load_from_file_impl(const ptree_type& cache)
{
	for (auto& [name, offset_packed]: cache)
	{
		const auto offset = offset_packed.get_value<uintptr_t>( );
		auto addr = base_address + offset;

		data_cache.emplace(const_cast<string&&>(name), move(addr));
	}

	return success;
}

void exports_storage::Change_base_address_impl(address new_addr)
{
	for (auto itr = data_cache.begin( ); itr != data_cache.end( ); ++itr)
	{
		auto& val = itr.value( );
		val -= (base_address);
		val += new_addr;
	}
}
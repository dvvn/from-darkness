#include "exports_storage.h"

using namespace nstd::os;

bool exports_storage::load_from_memory(cache_type& cache)
{
	const auto nt = this->nt_header( );

	// get export data directory.
	const auto data_dir = &nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
	if (!data_dir->VirtualAddress)
		return false;

	const auto base_address = this->base_addr( );

	// get export dir.
	const auto dir = base_address.add(data_dir->VirtualAddress).ptr<IMAGE_EXPORT_DIRECTORY>( );
#ifdef NDEBUG
    if (!dir)
        return 0;
#endif
	// names / funcs / ordinals ( all of these are RVAs ).
	const auto names = base_address.add(dir->AddressOfNames).ptr<uint32_t>( );
	const auto funcs = base_address.add(dir->AddressOfFunctions).ptr<uint32_t>( );
	const auto ords  = base_address.add(dir->AddressOfNameOrdinals).ptr<uint16_t>( );
#ifdef NDEBUG
    if (!names || !funcs || !ords)
        return 0;
#endif

	const auto all_modules = all_modules::get_ptr( );
	all_modules->update(false);

	cache_type temp_cache;

	// iterate names array.
	for (auto i = 0u; i < dir->NumberOfNames; ++i)
	{
		const std::string_view export_name = base_address.add(names[i]).ptr<const char>( );
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
			const std::string_view fwd_str = export_ptr.ptr<const char>( );

			// forwarders have a period as the delimiter.
			const auto delim = fwd_str.find_last_of('.');
			if (delim == fwd_str.npos)
				continue;

			// get forwarder mod name.
			const auto fwd_module       = fwd_str.substr(0, delim);
			const auto fwd_module_lower = ranges::views::transform(fwd_module, tolower);
			//const auto fwd_module_hash  = hashed_string_tag::_Compute_hash(fwd_module_lower, (".dll"));
			const auto fwd_module_str = std::string(fwd_module_lower.begin( ), fwd_module_lower.end( )).append(".dll");
			//const auto fwd_module_hash = module_info::create_hash(fwd_module_str);

			// get forwarder export name.
			const auto fwd_export = fwd_str.substr(delim + 1);

			// get real export ptr ( recursively ).
			//const auto target = _RANGES find(all_modules, fwd_module_hash, &module_info::name);

			auto target = all_modules->find(fwd_module_str);
			if (!target)
				continue;

			static_assert(!std::is_const_v<decltype(target)>, __FUNCSIG__": unable to preload cache!");

			auto& exports = target->exports( );
			exports.load( );
			const auto& exports_cache  = exports.get_cache( );
			const auto  fwd_export_ptr = exports_cache.find((fwd_export));
			if (fwd_export_ptr == exports_cache.end( ))
				continue;

			temp_cache.emplace(export_name, fwd_export_ptr->second);
		}
	}

	cache = move(temp_cache);
	//data_cache_.shrink_to_fit( );
	return true;
}

bool exports_storage::load_from_file(cache_type& cache, ptree_type&& storage)
{
	const auto base_address = this->base_addr( );

	for (auto itr = storage.begin( ); itr != storage.end( ); ++itr)
	{
		auto& name          = itr.key( );
		auto& offset_packed = itr.value( );

		const auto offset = offset_packed.get<uintptr_t>( );
		auto       addr   = base_address + offset;

		cache.emplace(const_cast<std::string&&>(name), std::move(addr));
	}

	return true;
}

bool exports_storage::read_to_storage(const cache_type& cache, ptree_type& storage) const
{
	const auto base_address = this->base_addr( );

	for (const auto& [name, addr]: cache)
	{
		storage[name] = addr.remove(base_address).value( );
	}

	return true;
}

//void exports_storage::Change_base_address_impl(address new_addr)
//{
//	for (auto itr = data_cache.begin( ); itr != data_cache.end( ); ++itr)
//	{
//		auto& val = itr.value( );
//		val -= (base_address);
//		val += new_addr;
//	}
//}

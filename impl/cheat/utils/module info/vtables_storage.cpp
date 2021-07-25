#include "vtables_storage.h"

#include "cheat/utils/memory block.h"
#include "cheat/utils/signature.h"
#include "cheat/utils/winapi/threads.h"

using namespace cheat;
using namespace utl;
using namespace detail;
using namespace property_tree;

//todo: add x64 support
static optional<vtable_info> _Load_vtable_info(const section_info& dot_rdata, const section_info& dot_text, address type_descriptor)
{
	for (const auto& block: dot_rdata.block.find_all_blocks(type_descriptor.value( )))
	{
		const auto xr = block.addr( );

		// so if it's 0 it means it's the class we need, and not some class it inherits from
		if (const auto vtable_offset = xr.remove(0x8).ref( ); vtable_offset != 0)
			continue;

		//NOTE1: find_block function rewritten, now it atomatically convert data to bytes if wanted
		//NOTE2: signature function rewritten, now if prefer span instead of vector
		//NOTE SUMMARY:
		//without signature we select hightest possible integral data type and compare with it
		//with signature we compare byte-to-byte or by memcmp

		// get the object locator
		const auto object_locator = xr - 0xC;
		const auto vtable_address = dot_rdata.block.find_block(object_locator).addr( ) + 0x4;

		// check is valid offset
		if (vtable_address <= 4U)
			continue;

		// get a pointer to the vtable

		// convert the vtable address to an ida pattern
		const auto temp_result = dot_text.block.find_block(/*signature*/vtable_address);

		//address not found
		if (temp_result.empty( ))
			continue;

		vtable_info info;
		info.addr = temp_result.addr( );
		return info;
	}

	return { };
}

vtables_storage::vtables_storage(address addr, size_t bytes_count, IMAGE_NT_HEADERS* nt, sections_storage* sections): data_cache_from_anywhere(move(addr), nt),
																													  bytes_count__(bytes_count),
																													  sections__(sections)
{
	load_mutex__ = make_shared<mutex>( );
}

void vtables_storage::set_sections(sections_storage* sections)
{
	sections__ = sections;
}

//currently only for x86
module_info_rw_result vtables_storage::Load_from_memory_impl( )
{
	const auto lock = utl::make_lock_guard(*load_mutex__);
	(void)lock;

#ifdef UTILS_X64
    throw std::runtime_error("todo: x64");
    ///look:
    ///https://github.com/samsonpianofingers/RTTIDumper/blob/master/
    ///https://www.google.com/search?q=rtti+typedescriptor+x64
#endif
	auto& sections = *sections__;
	sections.load_from_memory( );
	const auto& sections_cache = sections.get_cache( );
	const auto& dot_rdata = sections_cache.at(".rdata");
	const auto& dot_text = sections_cache.at(".text");

	using data_callback_fn = function<void(vtables_storage*)>;
	using future_type = shared_future<data_callback_fn>;
	auto load_data_storage = vector<future_type>( );

	static constexpr auto part_before = signature<BYTES>(".?AV");
	static constexpr auto part_after = signature<BYTES>("@@");

	// type descriptor names look like this: .?AVXXXXXXXXXXXXXXXXX@@ (so: ".?AV" + szTableName + "@@")

	//pause all other threads. we want all the power here;
	const auto frozen = winapi::frozen_threads_storage(true);
	(void)frozen;

	auto pool = thread_pool( );
	(void)pool;

	auto bytes = this->Mem_block( );
	while (true)
	{
		const auto block_start = bytes.find_block(part_before);
		if (block_start.empty( ))
			break;

		bytes = bytes.shift_to_end(block_start);
		switch (*block_start._Unchecked_end( ))
		{
			case ' ':
			case '\0':
			case '@':
			case '?':
			case '$':
			case '<':
				continue;
			default:
				break;
		}

		//--------------

		const auto block_end = bytes.find_block(part_after);
		if (block_end.empty( ))
			break;

		bytes = bytes.shift_to_end(block_end);

		//--------------

		const auto block_start_ptr = block_start._Unchecked_begin( );
		const auto block_end_ptr = block_end._Unchecked_end( );
		const auto block_size_raw = std::distance(block_start_ptr, block_end_ptr);

		const auto class_descriptor = string_view(reinterpret_cast<const char*>(block_start_ptr), block_size_raw);

#if 0
        //skip namespaces
        if (class_name.rfind(static_cast<uint8_t>('@')) != class_name.npos)
            continue;
#endif

		// get rtti type descriptor
		address type_descriptor = class_descriptor.data( );
		// we're doing - 0x8 here, because the location of the rtti typedescriptor is 0x8 bytes before the string
		type_descriptor -= 0x8;

		auto loader_fn = [&dot_rdata,&dot_text, class_descriptor, type_descriptor]( )-> data_callback_fn
		{
			auto result = _Load_vtable_info(dot_rdata, dot_text, type_descriptor);
			if (!result.has_value( ))
				return { };

			auto class_name = class_descriptor;
			class_name.remove_prefix(part_before.size( ));
			class_name.remove_suffix(part_after.size( ));

			return [class_name, info = move(*result)](vtables_storage* this_ptr) mutable
			{
				this_ptr->data_cache.emplace(string(class_name), move(info));
			};
		};
		load_data_storage.emplace_back(async(pool, move(loader_fn)));
	}

	for (auto& data: load_data_storage)
	{
		const auto callback = data.get( );
		if (callback.empty( ))
			continue;

		callback(this);
	}

	if (load_data_storage.empty( ))
		return nothing;
	if (!this->data_cache.empty( ))
		return success;

	return /*error*/nothing;
}

module_info_rw_result vtables_storage::Write_to_file_impl(ptree& cache) const
{
	for (const auto& [name, info]: data_cache)
	{
		ptree child;
		child.put("offset", info.addr.remove(base_address).value( ));
		//child.put("offset", info.addr - base_address_);
		//child.put("size", info.size( ));

		cache.add_child(name, child);
	}

	return success;
}

module_info_rw_result vtables_storage::Load_from_file_impl(const ptree& cache)
{
	for (auto& [name, child]: cache)
	{
		const auto offset = child.get<uintptr_t>("offset");
		//const auto size   = child.get<size_t>("size");

		vtable_info info;
		info.addr = base_address + offset;
		data_cache.emplace(const_cast<string&&>(name), move(info));
	}

	return success;
}

void vtables_storage::Change_base_address_impl(address new_addr)
{
	for (auto itr = data_cache.begin( ); itr != data_cache.end( ); ++itr)
	{
		auto& val = itr.value( ).addr;
		val -= (base_address);
		val += new_addr;
	}
}

memory_block vtables_storage::Mem_block( ) const
{
	return {base_address, bytes_count__};
}

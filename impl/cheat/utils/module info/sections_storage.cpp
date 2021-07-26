#include "sections_storage.h"

using namespace cheat;
using namespace utl;
using namespace utl::detail;
using namespace property_tree;

sections_storage::sections_storage(address addr, IMAGE_NT_HEADERS* nt): data_cache_from_memory(move(addr), nt)
{
}

module_info_rw_result sections_storage::Load_from_memory_impl( )
{
	const auto number_of_sections = nt->FileHeader.NumberOfSections;
	data_cache.reserve(number_of_sections);

	const auto section_header = IMAGE_FIRST_SECTION(nt);
	const auto last_section_header = section_header + number_of_sections;

	for (auto header = section_header; header != last_section_header; ++header)
	{
		auto& raw_name = header->Name;
		auto info_name = string(raw_name, ranges::find(raw_name, '\0'));

		section_info info;
		info.block = {base_address + header->VirtualAddress, header->SizeOfRawData};
		info.data = header;

		data_cache.emplace(move(info_name), move(info));
	}

	return success;
}

void sections_storage::Change_base_address_impl(address new_addr)
{
	for (auto itr = data_cache.begin( ); itr != data_cache.end( ); ++itr)
	{
		// ReSharper disable once CppUseStructuredBinding
		auto& info = itr.value( );

		address header_addr = info.data;
		header_addr -= (base_address);
		header_addr += new_addr;
		const auto header = header_addr.raw<IMAGE_SECTION_HEADER>( );

		info.block = {new_addr + header->VirtualAddress, header->SizeOfRawData};
		info.data = header;
	}
}

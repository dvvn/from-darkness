#include "sections_storage.h"

using namespace cheat;
using namespace utl;
using namespace utl::detail;
using namespace property_tree;

bool sections_storage::load_from_memory(cache_type& cache)
{
	const auto nt           = this->nt_header( );
	const auto base_address = this->base_addr( );

	cache_type tmp;

	const auto number_of_sections = nt->FileHeader.NumberOfSections;
	tmp.reserve(number_of_sections);

	const auto section_header      = IMAGE_FIRST_SECTION(nt);
	const auto last_section_header = section_header + number_of_sections;

	for (auto header = section_header; header != last_section_header; ++header)
	{
		auto& raw_name  = header->Name;
		auto  info_name = string(raw_name, ranges::find(raw_name, '\0'));

		section_info info;
		info.block = {base_address + header->VirtualAddress, header->SizeOfRawData};
		info.data  = header;

		tmp.emplace(move(info_name), move(info));
	}

	cache = move(tmp);
	return 1;
}

bool sections_storage::load_from_file(cache_type& cache, const ptree_type& storage)
{
	return 0;
}

bool sections_storage::read_to_storage(const cache_type& cache, ptree_type& storage) const
{
	return 0;
}

#if 0
void sections_storage::Change_base_address_impl(address new_addr)
{
	const auto base_address = this->base_addr( );

	for (auto itr = data_cache.begin( ); itr != data_cache.end( ); ++itr)
	{
		// ReSharper disable once CppUseStructuredBinding
		auto& info = itr.value( );

		address header_addr = info.data;
		header_addr -= (base_address);
		header_addr += new_addr;
		const auto header = header_addr.ptr<IMAGE_SECTION_HEADER>( );

		info.block = {new_addr + header->VirtualAddress, header->SizeOfRawData};
		info.data = header;
	}
}
#endif

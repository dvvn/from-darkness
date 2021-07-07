#include "data_cache.h"

using namespace cheat;
using namespace utl;
using namespace mem;
using namespace mem::detail;
using namespace property_tree;

data_cache_base::~data_cache_base( ) = default;

data_cache_base::data_cache_base(const address addr, IMAGE_NT_HEADERS* nt): base_address(addr),
																			nt(nt)
{
}

data_cache_result data_cache_base::load( )
{
	return load_from_memory( );
}

data_cache_result data_cache_base::load_from_memory( )
{
	if (!Cache_empty_impl( ))
		return data_cache_result::nothing;
	return Load_from_memory_impl( );
}

bool data_cache_base::change_base_address(address new_addr)
{
	if (base_address == new_addr)
		return false;
	if (!Cache_empty_impl( ))
		Change_base_address_impl(new_addr);
	base_address = new_addr;
	return true;
}

void data_cache_base::Empty_cache_assert( ) const
{
	BOOST_ASSERT_MSG(!Cache_empty_impl(), "Data cache is empty!");
}

data_cache_result data_cache_from_file::Load_from_file(data_cache_base& vtable1, const path_type& full_path)
{
	if (!vtable1.Cache_empty_impl( ))
		return data_cache_result::nothing;

	auto stream = std::basic_ifstream<char>(full_path.native( ));
	if (!stream)
		return data_cache_result::error;

	ptree_type tree;
	read_json(stream, tree);

	vtable1.Cache_reserve_impl(tree.size( ));
	return Load_from_file_impl(tree);
}

data_cache_result data_cache_from_file::Write_to_file(const data_cache_base& vtable1, const path_type& full_path) const
{
	if (vtable1.Cache_empty_impl( ))
		return data_cache_result::error;

	ptree_type tree;
	if (const auto write_result = Write_to_file_impl(tree); write_result != data_cache_result::success)
		return write_result;

	if (const auto folder = full_path.parent_path( ); !exists(folder) && !create_directories(folder))
		return data_cache_result::error;

	auto stream = std::basic_ofstream<ptree_type::key_type::value_type>(full_path.native( ));
	if (!stream)
		return data_cache_result::error;

	write_json(stream, tree);
	return data_cache_result::success;
}

data_cache_result data_cache_from_file::Load(data_cache_base& vtable1, const path_type& full_path)
{
	switch (Load_from_file(vtable1, full_path))
	{
		case data_cache_result::unknown:
		case data_cache_result::error:
		{
			switch (vtable1.load_from_memory( ))
			{
				case data_cache_result::success:
				{
					auto write_result = Write_to_file(vtable1, full_path);
					(void)write_result;
				}
				case data_cache_result::nothing:
					return data_cache_result::success;
				default:
					return data_cache_result::error;
			}
		}
		default:
			return data_cache_result::success;
	}
}

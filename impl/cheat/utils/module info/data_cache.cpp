#include "data_cache.h"

#include "cheat/utils/checksum.h"

using namespace cheat;
using namespace utl;
using namespace utl::detail;
using namespace property_tree;

address module_data_mgr_base::base_addr( ) const
{
	return root_class( )->base( );
}

IMAGE_NT_HEADERS* module_data_mgr_base::nt_header( ) const
{
	return root_class( )->NT( );
}

bool module_data_mgr_base::write_from_storage(const path_type& file, const ptree_type& storage) const
{
	if (storage.empty( ))
		return false;

	if (!exists(file))
	{
		create_directories(file.parent_path( ));
		write_json(file.string( ), storage);
	}
	else
	{
		auto mem_stream = std::ostringstream( );
		write_json(mem_stream, storage);

		string buff = mem_stream.str( );

		if (checksum(buff) != checksum(file))
		{
			std::ofstream stream(file);
			stream << (buff);
		}
	}

	return true;
}

bool module_data_mgr_base::read_to_storage(const path_type& file, ptree_type& storage) const
{
	if (!storage.empty( ))
		return false;
	if (!exists(file))
		return false;

	read_json(file.string( ), storage);
	return true;
}

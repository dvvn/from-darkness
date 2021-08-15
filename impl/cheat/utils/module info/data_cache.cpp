#include "data_cache.h"

using namespace cheat;
using namespace utl;
using namespace utl::detail;

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
		std::ofstream(file) << storage;
	}
	else
	{
		const auto buff = std::ostringstream( ) << storage;
		if (nstd::checksum(buff) != nstd::checksum(file))
			std::ofstream(file) << nstd::as_string(buff);
	}

	return true;
}

bool module_data_mgr_base::read_to_storage(const path_type& file, ptree_type& storage) const
{
	if (!storage.empty( ))
		return false;
	if (!exists(file))
		return false;

	std::ifstream(file) >> storage;
	return true;
}

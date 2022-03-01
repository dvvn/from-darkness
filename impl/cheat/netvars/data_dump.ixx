module;

#include "storage_includes.h"
//#include "cheat/console/includes.h"

#include <nstd/core.h>

#include <filesystem>

export module cheat.netvars.data_dump;
export import cheat.netvars.storage;
export import cheat.netvars.lazy;

export namespace cheat::netvars
{
	struct log_file_config
	{
		std::filesystem::path dir = NSTD_STRINGIZE_RAW_WIDE(NSTD_CONCAT(VS_SolutionDir, \.dumps\netvars\));
		size_t indent = 4;
		char filler = ' ';
		std::wstring extension = L".json";
	};

	bool log_netvars(const char* game_version, const storage& root_netvars_data, const log_file_config& cfg = {});
	void generate_classes(bool recreate, storage& root_netvars_data, lazy::files_storage& lazy_storage);
}
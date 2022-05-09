module;

#include <nstd/core_utils.h>

#include <vector>
#include <string>
#include <sstream>

export module cheat.netvars.core:storage;
import :basic_storage;
import cheat.csgo.structs.ClientClass;

export namespace cheat::netvars
{
	struct logs_data
	{
		~logs_data( );

		std::wstring dir = NSTD_STRINGIZE_RAW_WIDE(NSTD_CONCAT(VS_SolutionDir, \.dumps\netvars\));

		struct
		{
			std::wstring name;
			std::wstring extension = L".json";
		}file;

		size_t indent = 4;
		char filler = ' ';

		std::ostringstream buff;
	};

	struct classes_data
	{
		~classes_data( );

		std::wstring dir = NSTD_STRINGIZE_RAW_WIDE(NSTD_CONCAT(VS_SolutionDir, \impl\cheat\csgo\interfaces_custom\));

		struct file_info
		{
			std::wstring name;
			std::ostringstream buff;
		};

		std::vector<file_info> files;
	};

	class storage : public basic_storage
	{
	public:
		void iterate_client_class(csgo::ClientClass* const root_class) noexcept;
		void iterate_datamap(csgo::datamap_t* const root_map) noexcept;
		void store_handmade_netvars( ) noexcept;

		void log_netvars(logs_data& data) noexcept;
		void generate_classes(classes_data& data) noexcept;
	};
}

#pragma once

#include "cheat/core/service.h"

//#define CHEAT_NETVARS_UPDATING

#if defined(_DEBUG) || defined(CHEAT_NETVARS_UPDATING)
#define CHEAT_NETVARS_RESOLVE_TYPE
#endif
#if 0
#define CHEAT_NETVARS_DUMP_STATIC_OFFSET
#endif
#if defined(CHEAT_GUI_TEST) || (defined(CHEAT_NETVARS_DUMP_STATIC_OFFSET) && !defined(_DEBUG))
#define CHEAT_NETVARS_DUMPER_DISABLED
#endif

namespace cheat
{
	namespace detail
	{
		class lazy_file_writer final: public std::ostringstream
		{
		public:
			~lazy_file_writer( ) override;
			lazy_file_writer(std::filesystem::path&& file);

			lazy_file_writer(lazy_file_writer&& other);

		private:
			std::filesystem::path file_;
		};
	}

	class netvars final: public service<netvars>
#ifdef CHEAT_NETVARS_DUMPER_DISABLED
					   , service_always_skipped
#endif
	{
	public:
		~netvars( ) override;
		netvars( );

		int at(const std::string_view& table, const std::string_view& item) const;

		using data_type = nlohmann::json;
		using writers_storage_type = std::vector<detail::lazy_file_writer>;

	protected:
		load_result load_impl( ) override;
		void        after_load( ) override;

	private:
		writers_storage_type lazy_writer_;
		data_type            data_;
	};
}

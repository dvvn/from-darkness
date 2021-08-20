#pragma once

#include "cheat/core/service.h"

#include "cheat/sdk/datamap.hpp"
#include "cheat/sdk/Recv.hpp"

//#define CHEAT_NETVARS_UPDATING

namespace cheat
{
	class netvars final: public service<netvars>
	{
	public:
		~netvars( ) override;
		netvars( );

		int at(const std::string_view& table, const std::string_view& item) const;

	protected:
		bool load_impl( ) override;
		void after_load( ) override;

	private:
		enum class dump_info
		{
			skipped,
			created,
			updated
		};

		_NODISCARD dump_info Dump_netvars_( );
		void                 Generate_classes_(dump_info info);

		class lazy_file_writer final: public std::ostringstream
		{
		public:
			~lazy_file_writer( ) override;
			lazy_file_writer(std::filesystem::path&& file);

			lazy_file_writer(lazy_file_writer&& other) noexcept;

		private:
			std::filesystem::path file__;
		};

		using data_type = nlohmann::json;

		std::vector<lazy_file_writer> lazy_writer__;
		data_type                     data__;
	};
}

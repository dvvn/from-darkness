#pragma once

#include "cheat/core/service.h"

//#define CHEAT_NETVARS_UPDATING

namespace cheat
{
	namespace detail
	{
		class lazy_file_writer final: public std::ostringstream
		{
		public:
			~lazy_file_writer( ) override;
			lazy_file_writer(std::filesystem::path&& file);

			lazy_file_writer(lazy_file_writer&& other) noexcept;

		private:
			std::filesystem::path file_;
		};
	}

	class netvars final: public service<netvars>
	{
	public:
		~netvars( ) override;
		netvars( );

		int at(const std::string_view& table, const std::string_view& item) const;

		using data_type = nlohmann::json;
		using writers_storage_type = std::vector<detail::lazy_file_writer>;

	protected:
		bool load_impl( ) override;
		void after_load( ) override;

	private:
		writers_storage_type lazy_writer_;
		data_type           data_;
	};
}

#pragma once

#include "service.h"

#include "cheat/sdk/datamap.hpp"
#include "cheat/sdk/Recv.hpp"



namespace cheat
{
	class netvars final: public service_shared<netvars, service_mode::async>
	{
	public:
		~netvars( ) override;
		netvars( );

		int at(const utl::string_view& path) const;

	protected:
		void        Load( ) override;
		utl::string Get_loaded_message( ) const override;
		void        Post_load( ) override;

	private:
		class lazy_file_writer final: public std::stringstream, utl::noncopyable
		{
		public:
			~lazy_file_writer( ) override;
			lazy_file_writer(utl::filesystem::path&& file);

			lazy_file_writer(lazy_file_writer&& other) noexcept;

		private:
			utl::filesystem::path file__;
		};

		utl::vector<lazy_file_writer> lazy_writer__;
		utl::property_tree::ptree     data__;
	};
}

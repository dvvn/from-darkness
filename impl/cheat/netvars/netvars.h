#pragma once

#include "cheat/core/service.h"

#include "cheat/sdk/datamap.hpp"
#include "cheat/sdk/Recv.hpp"

namespace cheat
{
	class netvars final: public service<netvars>
	{
	public:
		~netvars( ) override;
		netvars( );

		int at(const utl::string_view& path) const;

	protected:
		bool Do_load( ) override;
		void On_load( ) override;

	private:
		void Dump_netvars_( );
		void Generate_classes_( );

		class lazy_file_writer final: public std::ostringstream, utl::noncopyable
		{
		public:
			~lazy_file_writer( ) override;
			lazy_file_writer(utl::filesystem::path&& file);

			lazy_file_writer(lazy_file_writer&& other) noexcept;

		private:
			utl::filesystem::path file__;
		};

		utl::vector<lazy_file_writer> lazy_writer__;
		utl::property_tree::ptree data__;
	};
}

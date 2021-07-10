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
		utl::property_tree::ptree data__;
	};
}

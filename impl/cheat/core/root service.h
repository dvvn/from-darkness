#pragma once
#include "service.h"

namespace cheat
{
	class root_service final: public service_static<root_service, service_mode::async>
	{
	public:
		root_service( );

#ifdef CHEAT_GUI_TEST
		void init( );
#else
		void init(HMODULE handle);
		//unhook current hook, exit current cutom thread, reset if wanted
		void    unload(BOOL ret = TRUE);
		HMODULE my_handle( ) const;
#endif

	protected:
		void Load( ) override;

	private:
	
#ifndef CHEAT_GUI_TEST
		HMODULE my_handle__ = nullptr;
#endif
	};
}

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
		auto init(HMODULE handle) -> void;
		//unhook current hook, exit current cutom thread, reset if wanted
		auto unload(BOOL ret = TRUE) -> void;
		auto my_handle( ) const -> HMODULE;
#endif

	protected:
		void Load( ) override;

	private:
#ifndef CHEAT_GUI_TEST
		HMODULE my_handle__ = nullptr;
#endif
	};
}

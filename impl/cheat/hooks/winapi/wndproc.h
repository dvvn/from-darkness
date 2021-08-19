#pragma once
#include "cheat/core/service.h"
#include "cheat/gui/objects/abstract page.h"

namespace cheat::hooks::winapi
{
	class wndproc final: public service<wndproc>,
						 public gui::objects::empty_page,
						 public decltype(_Detect_hook_holder(DefWindowProc))
	{
	public:
		wndproc( );

		void render( ) override;
	protected:
		bool Do_load( ) override;

		void callback(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) override;

		utl::address get_target_method_impl( ) const override;
	private:
		decltype(&DefWindowProc) default_wndproc__ = nullptr;
		bool unicode_=0;
		HWND hwnd_=0;

		bool    override_return__    = false;
		LRESULT override_return_to__ = 1;
	};
}

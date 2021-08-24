#pragma once
#include "cheat/core/service.h"
#include "cheat/gui/objects/abstract page.h"

namespace cheat::hooks::winapi
{
	class wndproc final: public service<wndproc>
					   , public dhooks::_Detect_hook_holder_t<decltype(DefWindowProc)>
					   , public gui::objects::empty_page
					   , service_hook_helper

	{
	public:
		void render( ) override;

	protected:
		bool load_impl( ) override;

		void callback(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) override;

		nstd::address get_target_method_impl( ) const override;
	private:
		decltype(&DefWindowProc) default_wndproc_ = nullptr;

		bool unicode_ = false;
		HWND hwnd_    = nullptr;

		bool    override_return_    = false;
		LRESULT override_return_to_ = TRUE;
	};
}

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

	


		void Callback(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) override;
	private:
		decltype(&DefWindowProc) default_wndproc__ = nullptr;

		bool override_return__ = false;
		LRESULT override_return_to__ = 1;
	};
}

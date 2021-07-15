#pragma once
#include "cheat/core/service.h"
#include "cheat/gui/menu/abstract page.h"

namespace cheat::hooks::input
{
	class wndproc final: public service_shared<wndproc, service_mode::async>,
						 public gui::menu::empty_page,
						 public decltype(detect_hook_holder(DefWindowProc))
	{
	public:
		wndproc( );

		void render( ) override;

	protected:
		void Load( ) override;
		void Callback(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) override;

	private:
		decltype(&DefWindowProc) default_wndproc__;

		bool override_return__ = false;
		LRESULT override_return_to__ = 1;
	};
}

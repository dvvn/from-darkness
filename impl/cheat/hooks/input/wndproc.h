#pragma once
#include "cheat/core/service.h"

namespace cheat::hooks::input
{
	class wndproc final: public service_shared<wndproc, service_mode::async>,
						 public decltype(detect_hook_holder(DefWindowProc))
	{
	public:
		wndproc( );

	protected:
		void Load( ) override;
		void Callback(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) override;

	private:
		decltype(&DefWindowProc) default_wndproc__;
	};
}

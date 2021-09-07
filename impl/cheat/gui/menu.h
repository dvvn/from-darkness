#pragma once

#include "cheat/core/service.h"

#include "objects/pages renderer.h"
#include "widgets/window.h"

// ReSharper disable CppInconsistentNaming
using UINT = unsigned int;
using UINT_PTR =
#ifdef _W64
_W64
#endif
unsigned
#if defined(_WIN64)
__int64
#else
int;
#endif
using WPARAM = UINT_PTR;
// ReSharper restore CppInconsistentNaming

#if defined(_DEBUG) ||  defined(CHEAT_GUI_TEST)
#define CHEAT_GUI_HAS_DEMO_WINDOW 1
#else
#define CHEAT_GUI_HAS_DEMO_WINDOW 0
#endif

namespace cheat::gui
{
	class menu final: public service<menu>, public widgets::window
	{
		using window::begin;
		using window::end;

	public:
		menu( );
		~menu( ) override;

		void render( );
		bool toggle(UINT msg, WPARAM wparam);

	protected:
		load_result load_impl( ) override;

	private:
		struct impl;
		std::unique_ptr<impl> impl_;
	};
}

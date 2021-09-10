#pragma once

#include "cheat/core/service.h"

// ReSharper disable CppInconsistentNaming
struct ImGuiContext;

struct HWND__;
using HWND = HWND__*;
// ReSharper restore CppInconsistentNaming

namespace cheat::gui
{
	class imgui_context final: public service<imgui_context>
	{
	public:
		~imgui_context( ) override;
		imgui_context( );

		HWND          hwnd( ) const;
		ImGuiContext& get( );

	protected:
		load_result load_impl( ) override;

	private:
		struct data_type;
		std::unique_ptr<data_type> data_;
	};
}

#pragma once

#include "cheat/core/service.h"

// ReSharper disable CppInconsistentNaming
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

		HWND hwnd( ) const;

	protected:
		load_result load_impl( ) override;

	private:
		HWND hwnd_ = nullptr;

		struct data_type;
		std::unique_ptr<data_type>data_;


	};
}

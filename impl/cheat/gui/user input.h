#pragma once
#include "cheat/core/service.h"

struct ImGuiContext;

namespace cheat::gui
{
	class user_input final: public service_shared< user_input,service_mode::async>
	{
	public:
		~user_input( ) override;
		user_input( );

		HWND hwnd( ) const;

	protected:
		void Load( ) override;

	public:
		enum class process_result
		{
			none,
			blocked,
			skipped
		};
		process_result process(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

	private:
		ImGuiContext* ctx__ = nullptr;
		HWND          hwnd__ = nullptr;
	};
}

#pragma once

#include "widget animator.h"

// ReSharper disable CppInconsistentNaming
struct ImVec2;
enum ImGuiWindowFlags_;
using ImGuiID = unsigned int;
using ImGuiWindowFlags = int;
// ReSharper restore CppInconsistentNaming

namespace cheat::gui::tools
{
	class imgui_string_transparent;
}

namespace cheat::gui::widgets
{
	class window: public content_background_fader
	{
	public:
		window(tools::animator&& fade = { });

		bool begin(tools::imgui_string_transparent&& title, ImGuiWindowFlags_ flags);
		void end( );

		void show( );
		void hide( );
		void toggle( );

		bool visible( ) const;
		bool active( ) const;

	private:
		bool ignore_end__ = false;
		bool visible__    = false;
	};

	class child_window: public content_background_fader
	{
	public:
		child_window(tools::animator&& fade = { });

		struct size_info
		{
			size_t count           = static_cast<size_t>(-1);
			float  biggest_element = FLT_MAX;

			enum
			{
				UNSET,
				WORD,
				RAW
			} type = UNSET;
		};

		bool begin(const ImVec2& size, bool border , ImGuiWindowFlags_ flags);

		void end( );

		void show( );

	protected:
		virtual bool Begin_impl(ImGuiID id, const ImVec2& size_arg, bool border, ImGuiWindowFlags extra_flags);
	};

	class child_frame_window: public child_window
	{
	public:
		child_frame_window(tools::animator&& fade = { });

	private:
		bool Begin_impl(ImGuiID id, const ImVec2& size_arg, bool border, ImGuiWindowFlags extra_flags) final;
	};
}

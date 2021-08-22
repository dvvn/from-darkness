#pragma once

#include "widget animator.h"

#include "cheat/gui/tools/string wrapper.h"

namespace cheat::gui::widgets
{
	struct window_flags final
	{
		using value_type = ImGuiWindowFlags_;

		NSTD_ENUM_STRUCT_BITFLAG(window_flags, ImGuiWindowFlags_None);
	};

	class window: public content_background_fader
	{
	public:
		window(tools::animator&& fade = { });

		bool begin(tools::prefect_string&& title, window_flags flags = { });
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
		bool begin(const size_info& size_info_x, const size_info& size_info_y, bool border = false, ImGuiWindowFlags_ flags = ImGuiWindowFlags_None);
		void end( );

		void show( );

	protected:
		virtual bool Begin_impl(ImGuiID id, const ImVec2& size_arg, bool border, ImGuiWindowFlags extra_flags);
	};

	class child_frame_window: public child_window
	{
	public:
		child_frame_window(tools::animator&& fade = { });

	protected:
		bool Begin_impl(ImGuiID id, const ImVec2& size_arg, bool border, ImGuiWindowFlags extra_flags) override;
	};
}

#pragma once
#include "cheat/gui/tools/animator.h"
#include "cheat/gui/tools/string wrapper.h"

namespace cheat::gui::widgets
{
	class window:public tools::widget_animator
	{
	public:
		window(tools::animator&& fade = { });

		bool begin(const tools::string_wrapper& title, ImGuiWindowFlags_ flags);
		void end( );

		void show( );
		void hide( );
		void toggle( );

		bool visible( ) const;
		bool active( ) const;

	private:
		bool ignore_end__ = false;
		bool visible__ = false;
	};

	class child_window:public tools::widget_animator
	{
	public:
		virtual ~child_window( ) = default;
		child_window(tools::animator&& fade = { });

		struct size_info
		{
			size_t count = static_cast<size_t>(-1);
			float biggest_element = FLT_MAX;

			enum
			{
				UNSET,
				WORD,
				RAW
			} type = UNSET;
		};
		bool begin(const size_info& size_info_x, const size_info& size_info_y, bool border = false, ImGuiWindowFlags_ flags = ImGuiWindowFlags_None);
		//bool begin(size_t size, ImGuiWindowFlags_ flags = ImGuiWindowFlags_None);
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

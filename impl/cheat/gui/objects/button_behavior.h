#pragma once

#include <cheat/gui/tools/callback.h>

#include <memory>

// ReSharper disable CppInconsistentNaming
typedef int ImGuiButtonFlags;
// ReSharper restore CppInconsistentNaming

namespace cheat::gui::objects
{
	class renderable;

	class button_behavior
	{
	public:
		button_behavior( );
		virtual ~button_behavior( );

		button_behavior(button_behavior&&) noexcept;
		button_behavior& operator=(button_behavior&&) noexcept;

		void set_button_flags(ImGuiButtonFlags flags);

		/*virtual*/
		ImGuiButtonFlags get_button_flags( ) const;

		/*struct callback_data_ex final: tools::callback_data
		{
			callback_data_ex(const callback_data& data);

			bool hovered = false;
			bool held    = false;
			bool pressed = false;
		};*/

		void add_pressed_callback(tools::callback_info&& info, tools::two_way_callback::ways way);
		void add_hovered_callback(tools::callback_info&& info, tools::two_way_callback::ways way);
		void add_held_callback(tools::callback_info&& info, tools::two_way_callback::ways way);

#ifdef CHEAT_GUI_CALLBACK_HAVE_INDEX
  		bool erase_pressed_callback(const tools::callback_id& ids, tools::two_way_callback::ways way);
		bool erase_hovered_callback(const tools::callback_id& ids, tools::two_way_callback::ways way);
		bool erase_held_callback(const tools::callback_id& ids, tools::two_way_callback::ways way);
#endif

	protected:
		struct button_callback_data
		{
			ImGuiID id      = -1;
			bool    hovered = false;
			bool    held    = false;
			bool    pressed = false;
		};

		//order: hovered, held, pressed
		void invoke_button_callbacks(const ImRect& rect, ImGuiID id) const;

		button_callback_data get_last_callback_data( ) const;

	private:
		struct impl;
		std::unique_ptr<impl> impl_;
	};
}

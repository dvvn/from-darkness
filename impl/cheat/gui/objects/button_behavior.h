#pragma once

#include <cheat/gui/tools/callback.h>

#include <memory>

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

		struct callback_data_ex final: tools::callback_data
		{
			callback_data_ex(const callback_data&data);

			bool hovered = false;
			bool held    = false;
			bool pressed = false;
		};

		void add_pressed_callback(tools::callback_info&& info, tools::two_way_callback::ways way);
		void add_hovered_callback(tools::callback_info&& info, tools::two_way_callback::ways way);
		void add_held_callback(tools::callback_info&& info, tools::two_way_callback::ways way);

	protected:
		//order: hovered, held, pressed
		void invoke_button_callbacks(const ImRect& rect, callback_data_ex& data) const;

	private:
		struct impl;
		std::unique_ptr<impl> impl_;
	};
}

#pragma once

#include <memory>

// ReSharper disable CppInconsistentNaming
struct ImRect;
using ImGuiID = unsigned int;
// ReSharper restore CppInconsistentNaming

namespace std
{
	template <class Fty>
	class function;
}

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

		struct callback_state
		{
			callback_state( );

			void tick( );

			size_t ticks = 0;
			double start;
			double duration = 0;
		};

		struct callback_data
		{
			renderable* caller;
			ImGuiID     id;

			bool hovered = false;
			bool held    = false;
			bool pressed = false;
		};

		using callback_type = std::function<void(const callback_data&, const callback_state&)>;

		void add_pressed_callback(callback_type&& callback, bool repeat);
		void add_hovered_callback(callback_type&& callback, bool repeat);
		void add_held_callback(callback_type&& callback, bool repeat);

		void add_unpressed_callback(callback_type&& callback, bool repeat);
		void add_unhovered_callback(callback_type&& callback, bool repeat);
		void add_unheld_callback(callback_type&& callback, bool repeat);

	protected:
		//order: hovered, held, pressed
		void invoke_callbacks(const ImRect& rect, callback_data& data) const;

	private:
		struct impl;
		std::unique_ptr<impl> impl_;
	};
}

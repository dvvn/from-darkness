#pragma once
#include "selectable.h"

#include <nstd/confirmable_value.h>

namespace cheat::gui::tools
{
	enum class button_state : int8_t;
}

namespace cheat::gui::widgets
{
	class checkbox : public selectable_bg
				   , public text
	{
	public:
		checkbox();
		~checkbox() override;

		checkbox(checkbox&&) noexcept;
		checkbox& operator=(checkbox&&) noexcept;

		enum state:uint8_t
		{
			STATE_IDLE
		  , STATE_SELECTED
		};

		void render() override;

		void set_check_color_modifier(std::unique_ptr<animation_property<ImVec4>>&& mod);

	protected:
		void render_check_mark(ImGuiWindow* window, const ImVec2& basic_pos, float basic_size);
		//ImGuiButtonFlags_ get_button_flags( ) const override;

	private:
		struct impl;
		std::unique_ptr<impl> impl_;
	};

	

	tools::button_state checkbox2(const tools::cached_text& label, /*nstd::confirmable_value<bool>*/bool& value
								, animation_property<ImVec4>* bg_animation    = 0
								, animation_property<ImVec4>* check_animation = 0);
}

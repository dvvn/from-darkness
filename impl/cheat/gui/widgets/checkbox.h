#pragma once
#include "selectable.h"

namespace cheat::gui::tools
{
	class string_wrapper;
}

namespace cheat::gui::widgets
{
	class checkbox: public selectable_bg
				  , public text
	{
	public:
		checkbox( );
		~checkbox( ) override;

		checkbox(checkbox&&) noexcept;
		checkbox& operator=(checkbox&&) noexcept;

		void render( ) override;

		void                       set_check_colors(std::unique_ptr<selectable_bg_colors_base>&& colors);
		selectable_bg_colors_base* get_check_colors( );

	protected:
		//ImGuiButtonFlags_ get_button_flags( ) const override;

	private:
		struct impl;
		std::unique_ptr<impl> impl_;
	};
}

#pragma once

#include "../objects/button_behavior.h"

namespace cheat::gui::widgets
{
	//not a selectable. base to selectable, checkbox, ant all same widgets
	class selectable_base: public objects::button_behavior
	{
	public:
		selectable_base( );
		~selectable_base( ) override;

		selectable_base(selectable_base&&) noexcept;
		selectable_base& operator=(selectable_base&&) noexcept;

		void select(const tools::callback_data& data);
		void deselect(const tools::callback_data& data);
		void toggle(const tools::callback_data& data);

		bool selected( ) const;

		void add_selected_callback(tools::callback_info&& info, tools::two_way_callback::ways way);

	private:
		struct impl;
		std::unique_ptr<impl> impl_;
	};
}

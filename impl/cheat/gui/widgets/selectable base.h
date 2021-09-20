#pragma once

#include "../objects/button_behavior.h"

namespace cheat::gui::widgets
{
	//not a selectable. base to selectable, checkbox, ant all same widgets
	class selectable_base : public objects::button_behavior
	{
	public:
		selectable_base();
		~selectable_base() override;

		selectable_base(selectable_base&&) noexcept;
		selectable_base& operator=(selectable_base&&) noexcept;

		void select();
		void deselect();
		void toggle();

		bool selected() const;

		enum state:uint8_t
		{
			STATE_IDLE
		  , STATE_SELECTED
		  , STATE_HOVERED
		  , STATE_HELD
		};

		void add_selected_callback(tools::callback_info&& info, tools::two_way_callback::ways way);
#ifdef CHEAT_GUI_CALLBACK_HAVE_INDEX
		bool erase_selected_callback(const tools::callback_id& ids, tools::two_way_callback::ways way);
#endif

	private:
		struct impl;
		std::unique_ptr<impl> impl_;
	};
}

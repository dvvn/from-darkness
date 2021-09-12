#pragma once

#include "selectable base.h"

#include "text.h"

#include <memory>

namespace std
{
	template <typename T>
	class function;
}

namespace cheat::gui::widgets
{
	class selectable: public selectable_base
					, public text
	{
	public:
		selectable( );
		~selectable( ) override;

		selectable(selectable&&) noexcept;
		selectable& operator=(selectable&&) noexcept;

		void render( ) override;
		//this->selected( ) ? this->deselect( ) : this->select( );
		using callback_type = std::function<void(selectable*)>;

		void add_pressed_callback(callback_type&& callback);

	private:
		struct data_type;
		std::unique_ptr<data_type> data_;
	};
}

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
	class selectable: public selectable_base2
					, public text

	{
	public:
		selectable( );
		~selectable( ) override;

		selectable(selectable&&) noexcept;
		selectable& operator=(selectable&&) noexcept;

		void render( ) override;

	private:
		struct data_type;
		std::unique_ptr<data_type> data_;
	};
}

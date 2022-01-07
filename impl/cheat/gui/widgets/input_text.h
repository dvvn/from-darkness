#pragma once
#if 0
#include "cheat/gui/objects/renderable object.h"

#include <memory>

namespace cheat::gui::tools
{
	class string_wrapper;
}

namespace cheat::gui::widgets
{
	class text: public objects::renderable
	{
	public:
		text();
		~text() override;
		void render( ) override;

		void set(tools::string_wrapper&& text);

	private:
		struct data;
		std::unique_ptr<data> data_;
	};
}
#endif
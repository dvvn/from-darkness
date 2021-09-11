#pragma once
#include "cheat/gui/objects/renderable object.h"
#include "cheat/gui/objects/shared_label.h"

struct ImFont;

namespace cheat::gui::tools
{
	class perfect_string;
	class string_wrapper;
}

namespace cheat::gui::widgets
{
	class text: public objects::renderable
	{
	public:
		text( );
		~text( ) override;

		void render( ) override;

		void set_font(ImFont* font);

		void set_label(tools::string_wrapper&& label); //todo: perfect_string ???
		void set_label(const objects::shared_label& label);

	private:
		struct data;
		std::unique_ptr<data> data_;
	};
}

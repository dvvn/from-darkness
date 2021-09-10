#pragma once
#include "cheat/gui/objects/renderable object.h"
#include "cheat/gui/objects/shared_label.h"

namespace cheat::gui::tools
{
	class string_wrapper;
}

namespace cheat::gui::widgets
{
	class text: public objects::renderable, public virtual objects::abstract_label
	{
	public:
		text( );
		~text( ) override;
		void render( ) override;
	};
}

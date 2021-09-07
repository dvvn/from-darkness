#pragma once

namespace cheat::gui::objects
{
	class renderable
	{
	public:
		virtual      ~renderable( ) = default;
		virtual void render( ) =0;
	};
}

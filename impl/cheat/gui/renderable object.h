#pragma once

namespace cheat::gui
{
	class renderable_object
	{
	public:
		virtual      ~renderable_object( ) = default;
		virtual void render( ) =0;
	};
}

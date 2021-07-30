#pragma once

namespace cheat::gui::objects
{
	class renderable_object
	{
	public:
		virtual      ~renderable_object( ) = default;
		virtual void render( ) =0;
	};
}
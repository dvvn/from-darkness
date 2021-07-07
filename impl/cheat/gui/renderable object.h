#pragma once

namespace cheat::gui
{
	class renderable_object
	{
	public:
		virtual      ~renderable_object( ) = default;
		virtual auto render( ) -> void =0;
	};
}

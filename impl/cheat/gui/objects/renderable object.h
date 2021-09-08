#pragma once

namespace std
{
	template <class T>
	class shared_ptr;
}

namespace cheat::gui::objects
{
	class renderable
	{
	public:
		virtual      ~renderable( ) = default;
		virtual void render( ) =0;
	};

	using renderable_shared = std::shared_ptr<renderable>;
}

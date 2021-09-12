#pragma once

struct ImGuiWindow;
typedef unsigned int ImGuiID;

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

		ImGuiID get_id( ) const;
		ImGuiID get_id( ImGuiWindow*wnd) const;
	};

	using renderable_shared = std::shared_ptr<renderable>;
}

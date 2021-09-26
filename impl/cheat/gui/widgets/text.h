#pragma once

#include "cheat/gui/objects/renderable object.h"
#include "cheat/gui/tools/cached_text.h"

// ReSharper disable CppInconsistentNaming
struct ImGuiWindow;
struct ImFont;
struct ImRect;
struct ImVec2;
// ReSharper restore CppInconsistentNaming

namespace cheat::gui::widgets
{
	//DEPRECATED!!!
	class text : public objects::renderable, public tools::cached_text
	{
	public:
		void render() override;

	protected:
		void render_text(ImGuiWindow* wnd, const ImVec2& pos);
		ImRect make_rect(ImGuiWindow* wnd) const;
	};
}

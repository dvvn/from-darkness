module;

#include <cheat/hooks/instance.h>

#include <imgui_internal.h>

module cheat.hooks.imgui.PushClipRect;
import cheat.gui;
import cheat.hooks.base;

using namespace cheat;
using namespace hooks;

CHEAT_HOOK_INSTANCE(imgui, PushClipRect);

static void* target( ) noexcept
{
	return ImGui::PushClipRect;
}

struct replace
{
	static void fn(const ImVec2& clip_rect_min, const ImVec2& clip_rect_max, bool intersect_with_current_clip_rect) noexcept
	{
		CHEAT_HOOK_CALL_ORIGINAL_STATIC(clip_rect_min, clip_rect_max, intersect_with_current_clip_rect);

		const auto wnd = ImGui::GetCurrentWindowRead( );

		//see ImGui::Begin
		//rendering starts after this line
		if (wnd->DrawList->CmdBuffer.Size != 1 || wnd->DrawList->CmdBuffer[0].ElemCount != 0)
			return;

		//todo: remove fallback window
		if (wnd->IsFallbackWindow)
			return;
		if (!gui::effects::is_applicable(wnd))
			return;

		const auto dlist = wnd->DrawList;
		const auto& rect = wnd->OuterRectClipped;
		dlist->PushClipRect(rect.Min, rect.Max);
		gui::effects::perform_blur(dlist, ImGui::GetStyle( ).Alpha);
		dlist->PopClipRect( );
	}
};

CHEAT_HOOK_INIT(imgui, PushClipRect);

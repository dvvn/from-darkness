module;

#include <imgui_internal.h>

module cheat.hooks.imgui:PushClipRect;
import cheat.gui;

using namespace cheat;
using namespace hooks::imgui;

PushClipRect::PushClipRect( )
{
	this->set_target_method(ImGui::PushClipRect);
}

void PushClipRect::callback(const ImVec2& clip_rect_min, const ImVec2& clip_rect_max, bool intersect_with_current_clip_rect)
{
	using namespace cheat::gui;

	const auto wnd = ImGui::GetCurrentWindowRead( );

	//see ImGui::Begin
	//rendering starts after this line
	if (wnd->DrawList->CmdBuffer.Size != 1 || wnd->DrawList->CmdBuffer[0].ElemCount != 0)
		return;

	//todo: remove fallback window
	if (wnd->IsFallbackWindow)
		return;
	if (!effects::is_applicable(wnd))
		return;

	this->call_original_and_store_result(clip_rect_min, clip_rect_max, intersect_with_current_clip_rect);

	const auto dlist = wnd->DrawList;
	const auto& rect = wnd->OuterRectClipped;
	dlist->PushClipRect(rect.Min, rect.Max);
	effects::perform_blur(dlist, ImGui::GetStyle( ).Alpha);
	dlist->PopClipRect( );
}
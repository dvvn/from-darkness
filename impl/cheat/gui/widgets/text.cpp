// ReSharper disable CppMemberFunctionMayBeConst
#include "text.h"

#include "cheat/gui/tools/cached_text.h"

#include "nstd/runtime assert.h"

#include <imgui_internal.h>

using namespace cheat::gui;
using namespace widgets;
using namespace objects;
using namespace tools;

void text::render()
{
	auto window = ImGui::GetCurrentWindow( );

	/*if (window->SkipItems)// uselles, window->begin already check it
		return;*/

	const auto bb = this->make_rect(window);

	//auto &g=*GImGui;
	//g.Font, g.FontSize DEPRECATED
	//ImGui::SetWindowFontScale DEPRECATED

	//g.LastItemData DEPRECATED
	/*if (!ImGui::ItemAdd(bb, 0))
		return;*/

	//g.LogEnabled DEPRECATED
	/*const bool is_clipped = ImGui::IsClippedEx(bb, id, false);
    if (is_clipped)
	   return false;*/

	/*
	 ///imgui::isclipped code

	ImGuiContext& g = *GImGui;
	ImGuiWindow* window = g.CurrentWindow;
	if (!bb.Overlaps(window->ClipRect))
		if (id == 0 || (id != g.ActiveId && id != g.NavId))
			if (clip_even_when_logged || !g.LogEnabled)
				return true;
	return false;
	 */

	//ItemAdd
	if (!bb.Overlaps(window->ClipRect))
		return;

	ImGui::ItemSize(this->get_label_size( ), 0.0f);
	render_text(window, bb.Min);

#if 0

#if CHEAT_GUI_HAS_IMGUI_STRV
	ImGui::TextUnformatted(l);
#else
	const auto mb = l.multibyte( );
	ImGui::TextUnformatted(mb._Unchecked_begin( ), mb._Unchecked_end( ));

#endif
#endif
}

void text::render_text(ImGuiWindow* wnd, const ImVec2& pos)
{
#if 0
	auto&& d = *this;
	auto&& l = d.label.multibyte( );

	//todo: own render text function without internal every-frame loops & shits
	wnd->DrawList->AddText(d.font, d.font->FontSize, pos, ImGui::GetColorU32(ImGuiCol_Text), l._Unchecked_begin( ), l._Unchecked_end( ));
#endif
	cached_text::render(wnd->DrawList, pos, ImGui::GetColorU32(ImGuiCol_Text));
}

ImRect text::make_rect(ImGuiWindow* wnd) const
{
	const auto& dc = wnd->DC;
	/*if (window->SkipItems)//todo: sheck from window->begin
		return;*/
	const auto& l_size = this->get_label_size( );
	const auto l_pos   = ImVec2(dc.CursorPos.x, dc.CursorPos.y + dc.CurrLineTextBaseOffset);

	return ImRect(l_pos, l_pos + l_size);
}

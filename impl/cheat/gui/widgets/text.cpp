#include "text.h"

#include "cheat/gui/tools/string wrapper.h"

#include "nstd/runtime assert.h"

#include <imgui_internal.h>

using namespace cheat::gui;
using namespace widgets;
using namespace objects;

struct text::data
{
	ImFont* font = ImGui::GetDefaultFont( ); //todo: shared_ptr. no push_pop font allowed

	shared_label label;
	ImVec2       label_size;
	bool         label_owned = false;
	//todo: color

	void set_font(ImFont* new_font)
	{
		font = new_font;
		update_label_size( );
	}

	void set_label(shared_label&& new_label, bool own)
	{
		runtime_assert(new_label != nullptr);
		label       = std::move(new_label);
		label_owned = own;
		update_label_size( );
	}

private:
	void update_label_size( )
	{
		if (!font || !label)
			return;

		auto&& l   = label->get_label( ).multibyte( );
		label_size = font->CalcTextSizeA(font->FontSize, FLT_MAX, 0.f, l._Unchecked_begin( ), l._Unchecked_end( ), nullptr);
	}
};

text::text( )
{
	data_ = std::make_unique<data>( );
}

text::~text( ) = default;

void text::render( )
{
	auto& d = *data_;

	auto  window = ImGui::GetCurrentWindow( );
	auto& dc     = window->DC;
	/*if (window->SkipItems)//todo: sheck from window->begin
		return;*/
	const auto& l      = d.label->get_label( ).multibyte( );
	const auto& l_size = d.label_size;
	const auto  l_pos  = ImVec2(dc.CursorPos.x, dc.CursorPos.y + dc.CurrLineTextBaseOffset);

	const auto bb = ImRect(l_pos, l_pos + l_size);
	ImGui::ItemSize(l_size, 0.0f); //do something with it

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

	if (!bb.Overlaps(window->ClipRect))
		return;

	//todo: own render text function without internal every-frame loops & shits
	window->DrawList->AddText(d.font, d.font->FontSize, bb.Min, ImGui::GetColorU32(ImGuiCol_Text), l._Unchecked_begin( ), l._Unchecked_end( ));

#if 0

#if CHEAT_GUI_HAS_IMGUI_STRV
	ImGui::TextUnformatted(l);
#else
	const auto mb = l.multibyte( );
	ImGui::TextUnformatted(mb._Unchecked_begin( ), mb._Unchecked_end( ));

#endif
#endif
}

void text::set_font(ImFont* font)
{
	data_->set_font(font);
}

void text::set_label(tools::string_wrapper&& label)
{
	data_->set_label(std::make_shared<non_abstract_label>((std::move(label))), true);
}

void text::set_label(const shared_label& label)
{
	auto copy = label;
	data_->set_label(std::move(copy), false);
}

// ReSharper disable once CppMemberFunctionMayBeConst
//void text::set_text(tools::string_wrapper&& text)
//{
//	static_cast<tools::string_wrapper&>(*data_) = std::move(text);
//}

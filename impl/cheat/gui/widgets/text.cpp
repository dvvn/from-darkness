#include "text.h"

#include "cheat/gui/tools/string wrapper.h"

#include "nstd/runtime assert.h"

#include <imgui_internal.h>

using namespace cheat::gui;
using namespace widgets;
using namespace objects;
using namespace tools;

struct text::data
{
	ImFont* font = ImGui::GetDefaultFont( ); //todo: shared_ptr. no push_pop font allowed

	string_wrapper label;
	ImVec2         label_size;
	//todo: color

	void set_font(ImFont* new_font)
	{
		font = new_font;
		update_label_size( );
	}

	void set_label(string_wrapper&& new_label)
	{
		label = std::move(new_label);
		update_label_size( );
	}

private:
	void update_label_size( )
	{
		if (!font || !label)
			return;

		auto&& l   = label.multibyte( );
		label_size = font->CalcTextSizeA(font->FontSize, FLT_MAX, 0.f, l._Unchecked_begin( ), l._Unchecked_end( ), nullptr);
	}
};

text::text( )
{
	data_ = std::make_unique<data>( );
}

text::~text( )                         = default;
text::text(text&&) noexcept            = default;
text& text::operator=(text&&) noexcept = default;

void text::render( )
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

	ImGui::ItemSize(data_->label_size, 0.0f);
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
	auto&& d = *data_;
	auto&& l = d.label.multibyte( );

	//todo: own render text function without internal every-frame loops & shits
	wnd->DrawList->AddText(d.font, d.font->FontSize, pos, ImGui::GetColorU32(ImGuiCol_Text), l._Unchecked_begin( ), l._Unchecked_end( ));
}

ImRect text::make_rect(ImGuiWindow* wnd) const
{
	auto& dc = wnd->DC;
	/*if (window->SkipItems)//todo: sheck from window->begin
		return;*/
	const auto& l_size = data_->label_size;
	const auto  l_pos  = ImVec2(dc.CursorPos.x, dc.CursorPos.y + dc.CurrLineTextBaseOffset);

	return ImRect(l_pos, l_pos + l_size);
}

void text::set_font(ImFont* font)
{
	data_->set_font(font);
}

void text::set_label(tools::string_wrapper&& label)
{
	data_->set_label(std::move(label));
}

const tools::string_wrapper& text::get_label( ) const
{
	return data_->label;
}

const ImVec2& text::label_size( ) const
{
	return data_->label_size;
}

// ReSharper disable once CppMemberFunctionMayBeConst
//void text::set_text(tools::string_wrapper&& text)
//{
//	static_cast<tools::string_wrapper&>(*data_) = std::move(text);
//}
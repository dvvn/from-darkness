#include "pages renderer.h"

#include "cheat/gui/imgui/push style var.h"

using namespace cheat;
using namespace gui;
using namespace imgui;
using namespace menu;
using namespace utl;

void pages_renderer::render( )
{
	const auto& style = ImGui::GetStyle( );

	constexpr auto dummy_text = string_view("W");
	const auto     sample_size = ImGui::CalcTextSize(dummy_text._Unchecked_begin( ), dummy_text._Unchecked_end( ));

	const auto frame_padding = style.FramePadding * 2.f;
	ImVec2     size;
	size.x = frame_padding.x +                 //space before and after
			 longest_string__ * sample_size.x; //reserve width for longest string

	size.y = frame_padding.y +                             //space before and after
			 objects_.size( ) * (sample_size.y) +          //all strings height						                            
			 style.ItemSpacing.y * (objects_.size( ) - 1); //space between all string

	//if (!ImGui::BeginListBox(title, size))
	//	return;
	if (!ImGui::BeginChildFrame(reinterpret_cast<ImGuiID>(this), size))
		return ImGui::EndChildFrame( );
	{
		const auto pop = push_style_var(ImGuiStyleVar_SelectableTextAlign, ImVec2(0.5f, 0.5f));
		(void)pop;

		for (auto& obj: objects_)
		{
			if (obj())
			{
				object_selected_->deselect( );
				object_selected_ = addressof(obj);
				obj.select( );
			}
		}
	}
	ImGui::EndChildFrame( );
	//ImGui::EndListBox( );

	ImGui::SameLine( );

	ImGui::BeginGroup( );
	{
		object_selected_->render( );
	}
	ImGui::EndGroup( );
}

void pages_renderer::init( )
{
	namespace r = ranges;
	namespace rv = ranges::views;

	abstract_pages_renderer::init( );
	object_selected_->select( );

	for (auto& p: objects_)
	{
		if (const auto obj = dynamic_cast<abstract_pages_renderer*>(p.page( )))
			obj->init( );
	}

	auto sizes = objects_ |
				 rv::transform(&abstract_page::name) |
				 rv::transform(&string_wrapper::raw) |
				 rv::transform(&wstring_view::size);
	longest_string__ = *r::max_element(sizes, std::less<size_t>( ));
}

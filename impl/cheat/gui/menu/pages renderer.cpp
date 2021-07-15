#include "pages renderer.h"

#include "cheat/gui/_imgui extension/push style var.h"

using namespace cheat;
using namespace gui;
using namespace imgui;
using namespace menu;
using namespace utl;

static auto _Sizes(const abstract_pages_renderer::container_type& container)
{
	return container
		   | ranges::views::transform(&abstract_page::name)
		   | ranges::views::transform(&string_wrapper::raw)
		   | ranges::views::transform(&wstring_view::size);
}

static ImVec2 _Char_size( )
{
	constexpr auto dummy_text = string_view("W");
	return ImGui::CalcTextSize(dummy_text._Unchecked_begin( ), dummy_text._Unchecked_end( ));
}

void vertical_pages_renderer::render( )
{
	const auto& style = ImGui::GetStyle( );
	const auto sample_size = _Char_size( );

	const auto frame_padding = style.FramePadding * 2.f;
	ImVec2 size;
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
			if (obj( ))
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

void vertical_pages_renderer::init( )
{
	abstract_pages_renderer::init( );

	auto sizes = _Sizes(objects_);
	longest_string__ = *ranges::max_element(sizes, std::less<size_t>( ));
}

void horizontal_pages_renderer::render( )
{
#if 0
	const string_wrapper tab_bar = "##" + to_string(reinterpret_cast<size_t>(this));
	if (!ImGui::BeginTabBar(tab_bar, ImGuiTabBarFlags_FittingPolicyResizeDown | ImGuiTabBarFlags_NoCloseWithMiddleMouseButton))
		return;

	const auto color_active_idx = ImGuiCol_TabActive;
	const auto color_inactive_idx = ImGuiCol_Tab;

	auto& color_active = ImGui::GetStyle( ).Colors[color_active_idx];
	auto& color_inactive = ImGui::GetStyle( ).Colors[color_inactive_idx];

	for (auto& [page]: objects_)
	{
		/*const auto anim_updated = p.animation.update( );

		if (anim_updated)
		{
			const auto dir = p.animation.dir( );
			const auto val = p.animation.value( );

			auto color = ImLerp(color_inactive, color_active, val);
			ImGui::PushStyleColor(dir == -1 ? color_inactive_idx : color_active_idx, color);
		}*/

		if (ImGui::BeginTabItem((page.name( )), nullptr, ImGuiTabItemFlags_NoTooltip | ImGuiTabItemFlags_NoReorder | ImGuiTabItemFlags_NoPushId))
		{
			//select_page(p, true);

			page->render( );

			ImGui::EndTabItem( );
		}

		//if (anim_updated)
		//	ImGui::PopStyleColor( );
	}
	ImGui::EndTabBar( );
#endif

	const auto& style = ImGui::GetStyle( );
	const auto sample_size = _Char_size( );

	//const auto indent_headers = style.ItemInnerSpacing.x;
	//const auto indent_page = style./*ItemInnerSpacing*/ItemSpacing.x; //ImGui use ItemSpacing by default

	const auto frame_padding = style.FramePadding * 2.f;
	ImVec2 size;
	size.x = frame_padding.x +                             //space before and after
			 /*indent_headers +*/                          //to indent first selectable
			 chars_count__ * sample_size.x +               //reserve width for all strings
			 style.ItemSpacing.x * (objects_.size( ) - 1); //space between all headers
	size.y = frame_padding.y + sample_size.y;

	if (!ImGui::BeginChildFrame(reinterpret_cast<ImGuiID>(this), size))
		return ImGui::EndChildFrame( );
	{
		const auto last_obj = addressof(objects_.back( ));
		//ImGui::SameLine(0, indent_headers); //for first selectable
		for (auto& obj: objects_)
		{
			const auto obj_ptr = addressof(obj);
			if (obj(ImGuiSelectableFlags_None, {wstring_view(obj.name( )).size( ) * sample_size.x, sample_size.y}))
			{
				object_selected_->deselect( );
				object_selected_ = obj_ptr;
				obj.select( );
			}

			if (obj_ptr != last_obj)
				ImGui::SameLine( );
		}
	}
	ImGui::EndChildFrame( );

	//ImGui::Indent(indent_page);
	//ImGui::BeginGroup();
	{
		object_selected_->render( );
	
	}
	//ImGui::EndGroup();
	//ImGui::Unindent(indent_page);
}

void horizontal_pages_renderer::init( )
{
	abstract_pages_renderer::init( );

	for (auto s: _Sizes(objects_))
		chars_count__ += s;
}

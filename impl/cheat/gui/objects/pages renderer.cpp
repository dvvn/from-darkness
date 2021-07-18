#include "pages renderer.h"

#include "cheat/gui/tools/push style var.h"

using namespace cheat;
using namespace gui;
using namespace tools;
using namespace objects;
using namespace utl;

static size_t _Get_num_chars(const pages_storage_data& page)
{
	return page.name( ).raw( ).size( );
}

static auto _Sizes(span<pages_storage_data>&& container)
{
	return container | ranges::views::transform(_Get_num_chars);
}

static ImVec2 _Char_size( )
{
	constexpr auto dummy_text = string_view("W");
	return ImGui::CalcTextSize(dummy_text._Unchecked_begin( ), dummy_text._Unchecked_end( ));
}

template <bool Sameline, class Fns>
static pages_storage_data* _Render_and_select(span<pages_storage_data>&& data, Fns&& size_getter)
{
	optional<pages_storage_data&> page_active;

	const auto data_begin = data.begin( );
	const auto data_end = data.end( );
	const auto pre_end = [&]( )-> std::_Span_iterator<pages_storage_data>
	{
		if constexpr (!Sameline)
			return { };
		else
			return std::prev(data_end);
	}( );

	const auto obj_invoke = [&](pages_storage_data& obj)
	{
		const ImVec2 size = invoke(size_getter, obj);
		return invoke(obj, ImGuiSelectableFlags_None, size);
	};

	for (auto itr = data_begin; itr != data_end; ++itr)
	{
		auto& obj = *itr;

		const auto obj_selected = obj.selected( );
		const auto obj_pressed = [&]
		{
			const auto ret = obj_invoke(obj);
			if constexpr (Sameline)
			{
				if (itr != pre_end)
					ImGui::SameLine( );
			}
			return ret;
		}( );

		if (obj_selected)
		{
			if (!page_active)
				page_active = obj;
			continue;
		}
		if (obj_pressed)
		{
			const auto itr_next = std::next(itr);

			if (!page_active.has_value( ))
			{
				auto selected_later = ranges::find_if(itr_next, data_end, &widgets::selectable_base::selected);
				BOOST_ASSERT(selected_later != data_end);
				page_active = *selected_later;
			}
			if (itr_next != data_end)
			{
				if constexpr (!Sameline)
				{
					ranges::for_each(itr_next, data_end, obj_invoke);
				}
				else
				{
					for (auto& p: span(itr_next, pre_end))
					{
						obj_invoke(p);
						ImGui::SameLine( );
					}

					obj_invoke(*pre_end);
				}
			}
			page_active->deselect( );
			page_active = obj;
			obj.select( );

			break;
		}
	}

	return page_active.get_ptr( );
}

void vertical_pages_renderer::render( )
{
	const auto& style = ImGui::GetStyle( );
	const auto sample_size = _Char_size( );

	const auto frame_padding = style.FramePadding * 2.f;
	ImVec2 size;
	size.x = frame_padding.x +                 //space before and after
			 longest_string__ * sample_size.x; //reserve width for longest string

	size.y = frame_padding.y +                          //space before and after
			 this->size( ) * sample_size.y +            //all strings height						                            
			 style.ItemSpacing.y * (this->size( ) - 1); //space between all string

	pages_storage_data* selected;

	if (!ImGui::BeginChildFrame(reinterpret_cast<ImGuiID>(this), size))
		return ImGui::EndChildFrame( );
	{
		const auto pop = push_style_var(ImGuiStyleVar_SelectableTextAlign, ImVec2(0.5f, 0.5f));
		(void)pop;

		selected = _Render_and_select<false>(*static_cast<vector*>(this), [](const abstract_page&) { return ImVec2(0, 0); });
	}
	ImGui::EndChildFrame( );

	ImGui::SameLine( );

	ImGui::BeginGroup( );
	{
		selected->render( );
	}
	ImGui::EndGroup( );
}

void vertical_pages_renderer::init( )
{
	abstract_pages_renderer::init( );

	auto sizes = _Sizes(*static_cast<vector*>(this));
	longest_string__ = *ranges::max_element(sizes, std::less<size_t>( ));
}

void horizontal_pages_renderer::render( )
{
	const auto& style = ImGui::GetStyle( );
	const auto sample_size = _Char_size( );

	//const auto indent_headers = style.ItemInnerSpacing.x;
	//const auto indent_page = style./*ItemInnerSpacing*/ItemSpacing.x; //ImGui use ItemSpacing by default

	const auto frame_padding = style.FramePadding * 2.f;
	ImVec2 size;
	size.x = frame_padding.x +                          //space before and after
			 /*indent_headers +*/                       //to indent first selectable
			 chars_count__ * sample_size.x +            //reserve width for all strings
			 style.ItemSpacing.x * (this->size( ) - 1); //space between all headers
	size.y = frame_padding.y + sample_size.y;

	pages_storage_data* selected;

	if (!ImGui::BeginChildFrame(reinterpret_cast<ImGuiID>(this), size))
		return ImGui::EndChildFrame( );
	{
		//ImGui::SameLine(0, indent_headers); //for first selectable
		selected = _Render_and_select<true>(*static_cast<vector*>(this), [&](const pages_storage_data& p)
		{
			return ImVec2(_Get_num_chars(p) * sample_size.x, 0);
		});
	}
	ImGui::EndChildFrame( );

	//ImGui::Indent(indent_page);
	//ImGui::BeginGroup();
	{
		selected->render( );
	}
	//ImGui::EndGroup();
	//ImGui::Unindent(indent_page);
}

void horizontal_pages_renderer::init( )
{
	abstract_pages_renderer::init( );

	for (auto s: _Sizes(*static_cast<vector*>(this)))
		chars_count__ += s;
}

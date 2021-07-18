#include "pages renderer.h"

#include "cheat/gui/tools/info.h"
#include "cheat/gui/tools/push style var.h"

#include "cheat/gui/widgets/window.h"

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

struct selected_info
{
	pages_storage_data* ptr;
	bool activated;
};

template <bool Sameline, class Fns>
static selected_info _Render_and_select(span<pages_storage_data>&& data, Fns&& size_getter)
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
			obj.select( );
			return {addressof(obj), true};
		}
	}
	return {page_active.get_ptr( ), false};
}

void vertical_pages_renderer::render( )
{
	selected_info selected;

	if (!this->begin({ }, {pages_.size( ), static_cast<float>(longest_string__), size_info::WORD}, true))
		return this->end( );
	{
		const auto pop = push_style_var(ImGuiStyleVar_SelectableTextAlign, ImVec2(0.5f, 0.5f));
		(void)pop;

		selected = _Render_and_select<false>(pages_, [](const abstract_page&) { return ImVec2(0, 0); });
	}
	this->end( );

	ImGui::SameLine( );

	if (selected.activated)
		selected_group__.show( );
	selected_group__.begin( );
	{
		selected.ptr->render( );
	}
	selected_group__.end( );
}

void vertical_pages_renderer::init( )
{
	abstract_pages_renderer::init( );

	auto sizes = _Sizes(pages_);
	longest_string__ = *ranges::max_element(sizes, std::less<size_t>( ));
}

void horizontal_pages_renderer::render( )
{
	selected_info selected;

	const auto char_size = _Get_char_size( ).x;

	size_info size;
#ifdef CHEAT_GUI_HORIZONTAL_PAGES_RENDERER_USE_LONGEST_STRING
	size = {pages_.size( ), static_cast<float>(longest_string__), size_info::WORD};
#else
	const auto extra_size = char_size / (ImGui::GetStyle( ).ItemInnerSpacing.x * pages_.size( ));
	size = {1, chars_count__ + extra_size, size_info::WORD};
#endif

	if (!this->begin(size, { }, true))
		return this->end( );
	{
		const auto pop = push_style_var(ImGuiStyleVar_SelectableTextAlign, ImVec2(0.5f, 0.5f));
		(void)pop;

		//ImGui::SameLine(0, indent_headers); //for first selectable
		selected = _Render_and_select<true>(pages_, [&](const pages_storage_data& p)
		{
			return ImVec2(
#ifdef CHEAT_GUI_HORIZONTAL_PAGES_RENDERER_USE_LONGEST_STRING
						  longest_string__
#else
						  _Get_num_chars(p)
#endif
						  * char_size, 0);
		});
	}
	this->end( );

	//ImGui::Indent(indent_page);
	if (selected.activated)
		selected_group__.show( );
	selected_group__.begin( );
	{
		selected.ptr->render( );
	}
	selected_group__.end( );
	//ImGui::Unindent(indent_page);
}

void horizontal_pages_renderer::init( )
{
	abstract_pages_renderer::init( );

	auto sizes = _Sizes(pages_);

#ifdef CHEAT_GUI_HORIZONTAL_PAGES_RENDERER_USE_LONGEST_STRING
	longest_string__ = *ranges::max_element(sizes, std::less<size_t>( ));
#else
	for (auto s: sizes)
		chars_count__ += s;
#endif
}

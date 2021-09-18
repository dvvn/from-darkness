// ReSharper disable CppMemberFunctionMayBeConst
#include "selectable.h"

#include "cheat/core/console.h"
#include "cheat/gui/tools/animator.h"
#include "cheat/gui/tools/string wrapper.h"

#include <nstd/overload.h>

#include <imgui_internal.h>

#include <array>
#include <format>
#include <functional>

using namespace cheat;
using namespace gui::widgets;
using namespace gui::tools;

void gui::widgets::detail::add_default_selectable_callbacks(selectable_bg* owner)
{
	using p = selectable_bg_colors_base::color_priority;
	using w = two_way_callback::ways;

	owner->add_hovered_callback(make_callback_info([=]
	{
		auto colors = owner->get_bg_colors( );
		colors->change_color(p::COLOR_HOVERED);
	}), w::WAY_TRUE);
	owner->add_hovered_callback(make_callback_info([=]
	{
		auto colors = owner->get_bg_colors( );
		colors->change_color(owner->selected( ) ? p::COLOR_SELECTED : p::COLOR_DEFAULT);
	}), w::WAY_FALSE);

	owner->add_held_callback(make_callback_info([=]
	{
		auto colors = owner->get_bg_colors( );
		colors->change_color(p::COLOR_HELD);
	}), w::WAY_TRUE);
	owner->add_held_callback(make_callback_info([=]
	{
		auto colors = owner->get_bg_colors( );
		colors->change_color(colors->get_last_color( ));
	}), w::WAY_FALSE);

	owner->add_selected_callback(make_callback_info([=]
	{
		auto colors = owner->get_bg_colors( );
		if (colors->get_current_color( ) != p::COLOR_DEFAULT)
			return;
		colors->change_color(p::COLOR_SELECTED);
	}), w::WAY_TRUE);
	owner->add_selected_callback(make_callback_info([=]
	{
		auto colors = owner->get_bg_colors( );
		if (colors->get_current_color( ) != p::COLOR_SELECTED)
			return;
		colors->change_color(p::COLOR_DEFAULT);
	}), w::WAY_FALSE);
}

//----

struct selectable_bg_colors_base::colors_updater::impl
{
	std::array<usnset_clr, 4> styles_map;
	std::array<ImVec4, 4>     colors;

private:
	template <size_t ...I>
	void update_style_impl(ImVec4* stcolors, std::index_sequence<I...>)
	{
		const auto change_color = [&](color_priority pr, const usnset_clr& idx)
		{
			//colors[pr] = idx.has_value( ) ? stcolors[*idx] : ImVec4{ };
			if (idx.has_value( ))
				colors[pr] = stcolors[*idx];
		};

		(change_color(static_cast<color_priority>(I), styles_map[I]), ...);
	}

public:
	void update_style(ImGuiStyle* style)
	{
		if (!style)
			style = std::addressof(ImGui::GetStyle( ));

		update_style_impl(style->Colors, std::make_index_sequence<4>( ));

		//colors[p::COLOR_DEFAULT]  = { }; //no color
		//		colors[p::COLOR_SELECTED] = clr[ImGuiCol_Header];
		//		colors[p::COLOR_HOVERED]  = clr[ImGuiCol_HeaderHovered];
		//		colors[p::COLOR_HELD]     = clr[ImGuiCol_HeaderActive];
	}
};

selectable_bg_colors_base::colors_updater::colors_updater( )
{
	impl_ = std::make_unique<impl>( );
}

selectable_bg_colors_base::colors_updater::~colors_updater( )                                                              = default;
selectable_bg_colors_base::colors_updater::colors_updater(colors_updater&&) noexcept                                       = default;
selectable_bg_colors_base::colors_updater& selectable_bg_colors_base::colors_updater::operator=(colors_updater&&) noexcept = default;

void selectable_bg_colors_base::colors_updater::set_style_indexes(usnset_clr&& def, usnset_clr&& selected, usnset_clr&& hovered, usnset_clr&& held)
{
	impl_->styles_map = {std::move(def), std::move(selected), std::move(hovered), std::move(held)};
}

void selectable_bg_colors_base::colors_updater::set_style_idx(color_priority priority, usnset_clr&& col)
{
	impl_->styles_map[priority] = std::move(col);
}

void selectable_bg_colors_base::colors_updater::change_color(color_priority priority, const ImVec4& color)
{
	impl_->colors[priority] = color;
}

void selectable_bg_colors_base::colors_updater::update_style(ImGuiStyle* style)
{
	impl_->update_style(style);
}

const ImVec4& selectable_bg_colors_base::colors_updater::operator[](color_priority idx) const
{
	return impl_->colors[idx];
}

//------------------

struct selectable_bg_colors_fade::impl
{
	impl( )
	{
		//todo: ---fade----
		//set time/min/max/dir once
		//
		fade.set(-1);
		fade.finish( );

		clr_last = clr_curr = COLOR_DEFAULT;
	}

	animator fade;

	color_priority clr_last, clr_curr;
	ImVec4         clr_from, clr_to;
	ImVec4         clr_tmp;

	void change_color(color_priority clr)
	{
		clr_last = clr_curr;
		clr_curr = clr;

		/*if (clr_last == clr_curr)
			return;*/

		runtime_assert(clr_last != clr_curr, "Logic error");

		const int8_t dir = clr_last < clr_curr ? 1 : -1;

		if (!fade.updating( ))
			clr_from = colors[clr_last];
		else
			clr_from = clr_tmp;

		clr_to = colors[clr_curr];
		fade.set(dir);
	}

	ImU32 calculate_color( )
	{
		//update_colors( );//for testing
		if (!fade.update( ))
			return ImGui::ColorConvertFloat4ToU32(clr_to);

		auto val = fade.value( );
		if (fade.dir( ) == -1)
			val = static_cast<animator::float_type>(1.0) - val;
		const auto diff = clr_to - clr_from;
		clr_tmp         = clr_from + diff * ImVec4(val, val, val, val);

		return ImGui::ColorConvertFloat4ToU32(clr_tmp);
	}

	colors_updater colors;

	//todo: if style changed, call this fucntion
	void update_colors( )
	{
		const auto clr_to_old = colors[clr_curr];

		colors.update_style(nullptr);

		// ReSharper disable once CppTooWideScopeInitStatement
		constexpr auto compare_colors = [](const ImVec4& a, const ImVec4& b)
		{
			return std::memcmp(&a, &b, sizeof(ImVec4)) == 0;
		};

		//if style changed, animate it
		if (!compare_colors(clr_to_old, colors[clr_curr]))
		{
			if (!fade.updating( ))
			{
				clr_from = clr_to_old;
				fade.restart( );
			}

			clr_to = colors[clr_curr];
		}
	}
};

selectable_bg_colors_fade::selectable_bg_colors_fade( )
{
	impl_ = std::make_unique<impl>( );
}

selectable_bg_colors_fade::~selectable_bg_colors_fade( )                                              = default;
selectable_bg_colors_fade::selectable_bg_colors_fade(selectable_bg_colors_fade&&) noexcept            = default;
selectable_bg_colors_fade& selectable_bg_colors_fade::operator=(selectable_bg_colors_fade&&) noexcept = default;

selectable_bg_colors_base::colors_updater& selectable_bg_colors_fade::get_colors_updater( )
{
	return (impl_->colors);
}

void selectable_bg_colors_fade::update_colors( )
{
	impl_->update_colors( );
}

void selectable_bg_colors_fade::change_color(color_priority clr)
{
	impl_->change_color(clr);
}

ImU32 selectable_bg_colors_fade::calculate_color( )
{
	return impl_->calculate_color( );
}

selectable_bg_colors_fade::color_priority selectable_bg_colors_fade::get_current_color( ) const
{
	return impl_->clr_curr;
}

selectable_bg_colors_fade::color_priority selectable_bg_colors_fade::get_last_color( ) const
{
	return impl_->clr_last;
}

animator& selectable_bg_colors_fade::fade( )
{
	return impl_->fade;
}

const animator& selectable_bg_colors_fade::fade( ) const
{
	return impl_->fade;
}

//-----------------

struct selectable_bg_colors_static::impl
{
	colors_updater colors;

	struct
	{
		ImU32          color;
		color_priority idx;
		//-
	} current_color;

	color_priority last_color;

	void change_color(color_priority clr)
	{
		last_color    = current_color.idx;
		current_color = {ImGui::ColorConvertFloat4ToU32(colors[clr]), clr};
	}
};

selectable_bg_colors_static::selectable_bg_colors_static( )
{
	impl_ = std::make_unique<impl>( );
}

selectable_bg_colors_static::~selectable_bg_colors_static( )                                                = default;
selectable_bg_colors_static::selectable_bg_colors_static(selectable_bg_colors_static&&) noexcept            = default;
selectable_bg_colors_static& selectable_bg_colors_static::operator=(selectable_bg_colors_static&&) noexcept = default;

selectable_bg_colors_base::colors_updater& selectable_bg_colors_static::get_colors_updater( )
{
	return (impl_->colors);
}

void selectable_bg_colors_static::update_colors( )
{
	impl_->colors.update_style(nullptr);
}

void selectable_bg_colors_static::change_color(color_priority clr)
{
	impl_->change_color(clr);
}

ImU32 selectable_bg_colors_static::calculate_color( )
{
	return impl_->current_color.color;
}

selectable_bg_colors_static::color_priority selectable_bg_colors_static::get_current_color( ) const
{
	return impl_->current_color.idx;
}

selectable_bg_colors_static::color_priority selectable_bg_colors_static::get_last_color( ) const
{
	return impl_->last_color;
}

//-----------------

struct selectable_bg::data_type
{
	std::unique_ptr<selectable_bg_colors_base> colors;
};

selectable_bg::selectable_bg( )
{
	data_ = std::make_unique<data_type>( );
	detail::add_default_selectable_callbacks(this);
}

selectable_bg::~selectable_bg( )                                  = default;
selectable_bg::selectable_bg(selectable_bg&&) noexcept            = default;
selectable_bg& selectable_bg::operator=(selectable_bg&&) noexcept = default;

void selectable_bg::set_bg_colors(std::unique_ptr<selectable_bg_colors_base>&& colors)
{
	colors->update_colors( );
	data_->colors = std::move(colors);
}

selectable_bg_colors_base* selectable_bg::get_bg_colors( )
{
	return data_->colors.get( );
}

bool selectable_bg::render(ImGuiWindow* window, ImRect& bb, ImGuiID id, bool outer_spacing)
{
	if (outer_spacing)
	{
		const auto& style     = ImGui::GetStyle( );
		const auto  spacing_x = /*span_all_columns ? 0.0f :*/ style.ItemSpacing.x;
		const auto  spacing_y = style.ItemSpacing.y;
		const auto  spacing_L = IM_FLOOR(spacing_x * 0.50f);
		const auto  spacing_U = IM_FLOOR(spacing_y * 0.50f);
		bb.Min.x -= spacing_L;
		bb.Min.y -= spacing_U;
		bb.Max.x += spacing_x - spacing_L;
		bb.Max.y += spacing_y - spacing_U;
	}

	/*if (!ImGui::ItemAdd(bb, id))
		return;*/
	if (!bb.Overlaps(window->ClipRect))
		return false;

	this->invoke_button_callbacks(bb, id);
	this->render_background(window, bb, this->get_bg_colors( ));

	//RenderNavHighlight(bb, id, ImGuiNavHighlightFlags_TypeThin | ImGuiNavHighlightFlags_NoRounding);
	return true;
}

void selectable_bg::render_background(ImGuiWindow* window, ImRect& bb, selectable_bg_colors_base* color)
{
	//ImGui::RenderFrame(bb.Min, bb.Max, col, false);
	window->DrawList->AddRectFilled(bb.Min, bb.Max, color->calculate_color( ));
}

//--------------

struct selectable::data_type
{
};

selectable::selectable( )
{
	data_ = std::make_unique<data_type>( );
}

selectable::~selectable( )                               = default;
selectable::selectable(selectable&&) noexcept            = default;
selectable& selectable::operator=(selectable&&) noexcept = default;

void selectable::render( )
{
	const auto window = ImGui::GetCurrentWindow( );

	auto bb = this->make_rect(window);
	ImGui::ItemSize(bb.GetSize( ));
	const auto text_pos = bb.Min;

	if (!selectable_bg::render(window, bb, this->get_id(window), true))
		return;

	this->render_text(window, text_pos);
}

// ReSharper disable CppMemberFunctionMayBeConst
#include "selectable.h"

#include "cheat/core/console.h"
#include "cheat/gui/tools/animator.h"
#include "cheat/gui/tools/push style color.h"
#include "cheat/gui/tools/string wrapper.h"

#include "nstd/overload.h"

#include <array>
#include <format>
#include <imgui_internal.h>

#include <functional>

using namespace cheat;
using namespace gui::widgets;
using namespace gui::tools;

struct selectable_exception final: std::exception
{
};

struct selectable_bg_colors_fade::impl
{
	std::array<ImVec4, 4> colors;

	impl( )
	{
		fade.set(-1);
		fade.finish( );

		clr_last = clr_curr = COLOR_DEFAULT;
	}

	animator fade;

	color_priority clr_last, clr_curr;
	ImVec4         clr_from, clr_to;
	ImVec4         clr_tmp;

	void set_from_to( )
	{
		if (clr_last == clr_curr)
			return;

		const auto dir = clr_last < clr_curr ? 1 : -1;

		if (!fade.updating( ))
			clr_from = colors[/*std::min(clr_last, clr_curr)*/clr_last];
		else
			clr_from = clr_tmp;

		clr_to = colors[/*std::max(clr_last, clr_curr)*/clr_curr];
		fade.set(dir);
	}

	void set_color(color_priority clr)
	{
		clr_last = clr_curr;
		clr_curr = clr;

		set_from_to( );
	}

	ImU32 get_color( )
	{
		if (!fade.update( ))
			return ImGui::ColorConvertFloat4ToU32(clr_to);

		auto val = fade.value( );
		if (fade.dir( ) == -1)
			val = 1.0 - val;
		const auto diff = clr_to - clr_from;
		clr_tmp         = clr_from + diff * ImVec4(val, val, val, val);

		return ImGui::ColorConvertFloat4ToU32(clr_tmp);
	}

	bool callbacks_set = false;

	void init( )
	{
		const auto& clr = ImGui::GetStyle( ).Colors;

		colors[COLOR_DEFAULT]  = { }; //no color
		colors[COLOR_SELECTED] = clr[ImGuiCol_Header];
		colors[COLOR_HOVERED]  = clr[ImGuiCol_HeaderHovered];
		colors[COLOR_HELD]     = clr[ImGuiCol_HeaderActive];

		if (!callbacks_set)
			return;

		set_from_to( );
	}

	//--------

	struct color_proxy
	{
		using fn_type = std::function<color_priority(selectable_bg*)>;
		using fn_type_ex = std::function<color_priority(selectable_bg*, const callback_data&, const callback_state&)>;

		template <typename T>
		color_proxy(T&& obj)
			: getter_(std::forward<T>(obj))
		{
		}

		color_priority get(selectable_bg* caller, const callback_data& data, const callback_state& state) const
		{
			return std::visit(nstd::overload(
				[](const color_priority clr) { return clr; },
				[=](const fn_type&      fn) { return fn(caller); },
				[&](const fn_type_ex&   fn) { return fn(caller, data, state); }
				), getter_);
		}

	private:
		std::variant<color_priority, fn_type, fn_type_ex> getter_;
	};
};

selectable_bg_colors_fade::selectable_bg_colors_fade( )
{
	impl_ = std::make_unique<impl>( );
}

selectable_bg_colors_fade::~selectable_bg_colors_fade( )                                              = default;
selectable_bg_colors_fade::selectable_bg_colors_fade(selectable_bg_colors_fade&&) noexcept            = default;
selectable_bg_colors_fade& selectable_bg_colors_fade::operator=(selectable_bg_colors_fade&&) noexcept = default;

void selectable_bg_colors_fade::init_colors(selectable_bg* owner)
{
	impl_->init( );

	auto& cb_set = impl_->callbacks_set;

	if (cb_set)
		return;
	cb_set = true;

	static constexpr auto build_callback = [](const std::string_view& debug_name, impl::color_proxy&& proxy)
	{
		auto fn =
			[wdebug_name = std::wstring(debug_name.begin( ), debug_name.end( )), color = std::move(proxy)]
		(const callback_data& data, const callback_state& state)
		{
			const auto caller = dynamic_cast<selectable_bg*>(data.caller);
			runtime_assert(caller != nullptr);
			//const auto caller      = static_cast<selectable_bg*>(data.caller);
			auto& caller_data = *caller->get_bg_colors( );
			try
			{
				caller_data.set_color(color.get(caller, data, state));
			}
			catch (const selectable_exception&)
			{
				// ReSharper disable once CppRedundantControlFlowJump
				return;
			}
			catch (...)
			{
				throw;
			}

#if 0
			CHEAT_CONSOLE_LOG(std::format(L""
				"selectable_bg \"{}\". callback \"{}\".\n"
				"  state     : start: {}, ticks: {}, duration: {}\n"
				"  color     : changed from \"{}\" to \"{}\"\n"
				"  animation : dir: {}, value: {}"
			  , caller->get_label( ).raw( ), wdebug_name
			  , state.start, state.ticks, state.duration
			  , static_cast<uint8_t>(caller_data.clr_last), static_cast<uint8_t>(caller_data.clr_curr)
			  , caller_data.fade.dir( ), caller_data.fade.value( )));
#endif
		};

		return callback_info(std::move(fn), false);
	};

	owner->add_hovered_callback(build_callback("hovered", COLOR_HOVERED)
	  , two_way_callback::WAY_TRUE);
	owner->add_hovered_callback(build_callback("unhovered", [](const selectable_bg* sel)
		{
			return sel->selected( ) ? COLOR_SELECTED : COLOR_DEFAULT;
		})
	  , two_way_callback::WAY_FALSE);

	owner->add_held_callback(build_callback("held", COLOR_HELD)
	  , two_way_callback::WAY_TRUE);
	owner->add_held_callback(build_callback("unheld", [](selectable_bg* sel)
		{
			const auto colors0 = sel->get_bg_colors( );
			runtime_assert(dynamic_cast<selectable_bg_colors_fade*>(colors0) != nullptr);
			const auto& colors = *static_cast<selectable_bg_colors_fade*>(colors0);

			return colors.impl_->clr_last;
		})
	  , two_way_callback::WAY_FALSE);

	owner->add_selected_callback(build_callback("selected",
			[](selectable_bg* sel)
			{
				const auto colors0 = sel->get_bg_colors( );
				runtime_assert(dynamic_cast<selectable_bg_colors_fade*>(colors0) != nullptr);
				const auto& colors = *static_cast<selectable_bg_colors_fade*>(colors0);

				if (colors.impl_->clr_curr != COLOR_DEFAULT) //avoid when hovered or presses
					throw selectable_exception( );
				return COLOR_SELECTED;
			})
	  , two_way_callback::WAY_TRUE);
	owner->add_selected_callback(build_callback("deselected", COLOR_DEFAULT)
	  , two_way_callback::WAY_FALSE);
}

void selectable_bg_colors_fade::set_color(color_priority clr)
{
	impl_->set_color(clr);
}

ImU32 selectable_bg_colors_fade::get_color( )
{
	return impl_->get_color( );
}

//-----------------

struct selectable_bg::data_type
{
	std::unique_ptr<selectable_bg_colors_base> colors;
};

selectable_bg::selectable_bg( )
{
	data_ = std::make_unique<data_type>( );
}

selectable_bg::~selectable_bg( )                                  = default;
selectable_bg::selectable_bg(selectable_bg&&) noexcept            = default;
selectable_bg& selectable_bg::operator=(selectable_bg&&) noexcept = default;

void selectable_bg::set_bg_colors(std::unique_ptr<selectable_bg_colors_base>&& colors)
{
	data_->colors = std::move(colors);
}

selectable_bg_colors_base* selectable_bg::get_bg_colors( )
{
	return data_->colors.get( );
}

bool selectable_bg::render(ImGuiWindow* window, ImRect& bb, callback_data_ex& cb_data, bool outer_spacing)
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

	this->invoke_button_callbacks(bb, cb_data);

	//ImGui::RenderFrame(bb.Min, bb.Max, col, false);
	window->DrawList->AddRectFilled(bb.Min, bb.Max, data_->colors->get_color( ));

	//RenderNavHighlight(bb, id, ImGuiNavHighlightFlags_TypeThin | ImGuiNavHighlightFlags_NoRounding);
	return true;
}

//--------------

struct selectable::data_type
{
};

selectable::selectable( )
{
	data_ = std::make_unique<data_type>( );

	auto bg_colors = std::make_unique<selectable_bg_colors_fade>( );
	bg_colors->init_colors(this);
	this->set_bg_colors(std::move(bg_colors));
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

	const auto id      = this->get_id(window);
	auto       cb_data = callback_data_ex(callback_data(this, id));

	if (!selectable_bg::render(window, bb, cb_data, true))
		return;

	this->render_text(window, text_pos);
}

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

struct selectable::data_type
{
	enum color_priority :uint8_t
	{
		COLOR_DEFAULT
	  , COLOR_SELECTED
	  , COLOR_HOVERED
	  , COLOR_HELD
	};

	std::array<ImVec4, 4> colors;

	data_type( )
	{
		fade.set(-1);
		fade.finish( );

		auto& clr = ImGui::GetStyle( ).Colors;

		colors[COLOR_DEFAULT]  = { }; //no color
		colors[COLOR_SELECTED] = clr[ImGuiCol_Header];
		colors[COLOR_HOVERED]  = clr[ImGuiCol_HeaderHovered];
		colors[COLOR_HELD]     = clr[ImGuiCol_HeaderActive];

		clr_last = clr_curr = COLOR_DEFAULT;
	}

	tools::animator fade;

	color_priority clr_last, clr_curr;
	ImVec4         clr_from, clr_to;
	ImVec4         clr_tmp;

	void set_color(color_priority clr)
	{
		clr_last = clr_curr;
		clr_curr = clr;

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

	ImU32 get_color( )
	{
		if (!fade.update( ))
			return ImGui::ColorConvertFloat4ToU32(colors[clr_curr]);

		auto val = fade.value( );
		if (fade.dir( ) == -1)
			val = 1.0 - val;
		const auto diff = clr_to - clr_from;
		clr_tmp         = clr_from + diff * ImVec4(val, val, val, val);

		return ImGui::ColorConvertFloat4ToU32(clr_tmp);
	}
};

selectable::selectable( )
{
	data_ = std::make_unique<data_type>( );

	struct color_proxy
	{
		using fn_type = std::function<data_type::color_priority(const selectable*)>;
		using fn_type_ex = std::function<data_type::color_priority(const selectable*, const callback_data&, const callback_state&)>;

		color_proxy(data_type::color_priority color)
			: getter_(color)
		{
		}

		color_proxy(fn_type&& fn)
			: getter_(std::move(fn))
		{
		}

		color_proxy(fn_type_ex&& fn)
			: getter_(std::move(fn))
		{
		}

		data_type::color_priority get(const selectable* caller, const callback_data& data, const callback_state& state) const
		{
			return std::visit(nstd::overload(
				[](data_type::color_priority clr) { return clr; },
				[=](const fn_type&           fn) { return fn(caller); },
				[&](const fn_type_ex&        fn) { return fn(caller, data, state); }
				), getter_);
		}

	private:
		std::variant<data_type::color_priority, fn_type, fn_type_ex> getter_;
	};

	struct fake_exception: std::exception
	{
	};

	static constexpr auto build_callback = [](const std::string_view& debug_name, color_proxy&& proxy)
	{
		auto fn =
			[wdebug_name = std::wstring(debug_name.begin( ), debug_name.end( )), color = std::move(proxy)]
		(const callback_data& data, const callback_state& state)
		{
			runtime_assert(dynamic_cast<selectable*>(data.caller) != nullptr);
			const auto caller      = static_cast<selectable*>(data.caller);
			auto&      caller_data = *caller->data_;
			try
			{
				caller_data.set_color(color.get(caller, data, state));
			}
			catch (const fake_exception&)
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
				"selectable \"{}\". callback \"{}\".\n"
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

	this->add_hovered_callback(build_callback("hovered", (data_type::COLOR_HOVERED))
	  , two_way_callback::WAY_TRUE);
	this->add_hovered_callback(build_callback("unhovered", color_proxy([](const selectable* sel) { return sel->selected( ) ? data_type::COLOR_SELECTED : data_type::COLOR_DEFAULT; }))
	  , two_way_callback::WAY_FALSE);

	this->add_held_callback(build_callback("held", (data_type::COLOR_HELD))
	  , two_way_callback::WAY_TRUE);
	this->add_held_callback(build_callback("unheld", color_proxy([](const selectable* sel) { return sel->data_->clr_last; }))
	  , two_way_callback::WAY_FALSE);

	this->add_selected_callback(build_callback("selected",
			color_proxy([](const selectable* sel)
			{
				if (sel->data_->clr_curr != data_type::COLOR_DEFAULT) //avoid when hovered or presses
					throw fake_exception( );
				return (data_type::COLOR_SELECTED);
			}))
	  , two_way_callback::WAY_TRUE);
	this->add_selected_callback(build_callback("deselected", (data_type::COLOR_DEFAULT))
	  , two_way_callback::WAY_FALSE);
}

selectable::~selectable( )                               = default;
selectable::selectable(selectable&&) noexcept            = default;
selectable& selectable::operator=(selectable&&) noexcept = default;

void selectable::render( )
{
	const auto  window = ImGui::GetCurrentWindow( );
	const auto& style  = ImGui::GetStyle( );

	/*if (window->SkipItems)// uselles, window->begin already check it
		return;*/

	auto bb = this->make_rect(window);
	ImGui::ItemSize(bb.GetSize( ));
	const auto text_pos = bb.Min;

	const float spacing_x = /*span_all_columns ? 0.0f :*/ style.ItemSpacing.x;
	const float spacing_y = style.ItemSpacing.y;
	const float spacing_L = IM_FLOOR(spacing_x * 0.50f);
	const float spacing_U = IM_FLOOR(spacing_y * 0.50f);
	bb.Min.x -= spacing_L;
	bb.Min.y -= spacing_U;
	bb.Max.x += spacing_x - spacing_L;
	bb.Max.y += spacing_y - spacing_U;

	const auto id = this->get_id(window);

	/*if (!ImGui::ItemAdd(bb, id))
		return;*/
	if (!bb.Overlaps(window->ClipRect))
		return;

	//data_->reset_colors( );
	//
	//auto& [bg_color,fade/*,bg_color_override*/] = *data_;

	auto cb_data = callback_data_ex(callback_data(this, id));
	this->invoke_button_callbacks(bb, cb_data);

	//this->Animate( ); //DEPRECATED

#if 0
	if(this->Animate( ))
	{
		auto clr = bg_color.has_value( ) ? ImGui::ColorConvertU32ToFloat4(*bg_color) : ImGui::GetStyleColorVec4(ImGuiCol_Header);
		clr.w *= this->Anim_value( );
		bg_color/*_override*/ = ImGui::ColorConvertFloat4ToU32(clr);
	}
	else if(this->selected( ))
	{
		bg_color = ImGui::GetColorU32(ImGuiCol_Header);
		//--
	}
#endif

#if 0
	const auto animating = this->Animate( );
	if(!bg_color.has_value( ))
	{
		if(animating)
		{
			auto clr = ImGui::GetStyleColorVec4(ImGuiCol_Header);
			clr.w *= this->Anim_value( );
			bg_color/*_override*/ = ImGui::ColorConvertFloat4ToU32(clr);
		}
		else if(this->selected( ))
		{
			bg_color = ImGui::GetColorU32(ImGuiCol_Header);
			//--
		}
	}
#endif

	//ImGui::RenderFrame(bb.Min, bb.Max, col, false);
	//if (bg_color.has_value( ))
	//	window->DrawList->AddRectFilled(bb.Min, bb.Max, *bg_color);

	window->DrawList->AddRectFilled(bb.Min, bb.Max, data_->get_color( ));

	this->render_text(window, text_pos);

	//RenderNavHighlight(bb, id, ImGuiNavHighlightFlags_TypeThin | ImGuiNavHighlightFlags_NoRounding);

	//text::render( );
}

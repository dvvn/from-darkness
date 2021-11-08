#include "menu.h"

#include "imgui_context.h"

#include "cheat/core/console.h"
#include "cheat/core/services_loader.h"
#include "cheat/features/aimbot.h"

#include "tools/cached_text.h"
#include "widgets/window.h"

#include <nstd/runtime_assert_fwd.h>

#include <imgui.h>
#include <imgui_internal.h>

#include <cppcoro/task.hpp>

#include <Windows.h>

#include <algorithm>
#include <functional>
#include <random>
#include <filesystem>

using namespace cheat::gui;
using namespace tools;
using namespace widgets;

using namespace std::chrono_literals;

struct menu_impl::impl
{
	WPARAM hotkey = VK_HOME;

	window_wrapped menu_window;

	void init_pages( )
	{
#if 0
		tabs_pages.make_vertical();
		tabs_pages.make_size_static();

		/*constexpr auto init_selectable_colors = []
		{
			using p = selectable_bg_colors_base::color_priority;

			auto updater = selectable_bg_colors_base::colors_updater( );
			updater.set_style_idx(p::COLOR_DEFAULT, { });
			updater.set_style_idx(p::COLOR_SELECTED, ImGuiCol_Header);
			updater.set_style_idx(p::COLOR_HOVERED, ImGuiCol_HeaderHovered);
			updater.set_style_idx(p::COLOR_HELD, ImGuiCol_HeaderActive);

			auto bg            = std::make_unique<selectable_bg_colors_dynamic>( );
			bg->clr_updater( ) = std::move(updater);

			return bg;
		};*/
		constexpr auto make_pressed_callback = [](tab_bar* source, tab_bar_item* self)-> callback_info
		{
			auto fn = [=]
			{
				const auto selected_before = source->get_selected();
				if (selected_before == self)
					return;

				selected_before->deselect();
				self->select();
			};

			return make_callback_info(std::move(fn), false);
		};

		constexpr auto add_item_set_callbacks = [&] <typename C, size_t S, typename T>(tab_bar_with_pages & tab_bar, const C(&name)[S], const std::shared_ptr<T>&data)-> T&
		{
			using namespace std::chrono_literals;

			auto item = std::make_unique<tab_bar_item>();

			item->set_font(ImGui::GetDefaultFont());
			item->set_label((name));
			auto anim = std::make_unique<smooth_value_linear<ImVec4>>();
			anim->set_duration(400ms);
			item->set_background_color_modifier(std::move(anim));
			item->add_pressed_callback(make_pressed_callback(std::addressof(tab_bar), item.get()), two_way_callback::WAY_TRUE);

			tab_bar.add_item(std::move(item), data);

			return *data;
		};

		auto& rage_tab = add_item_set_callbacks(tabs_pages, "rage", std::make_shared<tab_bar_with_pages>());

		rage_tab.make_horisontal();
		rage_tab.make_size_auto();
		add_item_set_callbacks(rage_tab, "aimbot", features::aimbot::get_ptr_shared());
		add_item_set_callbacks(rage_tab, "aimbot1", features::aimbot::get_ptr_shared());
		add_item_set_callbacks(rage_tab, "aimbot2", features::aimbot::get_ptr_shared());
#endif
	}

	void init_window( )
	{
		cached_text::label_type title_str;

		const auto title_append = [&]<class ...T>(T&& ...texts)
		{
			(title_str.append(std::forward<T>(texts)), ...);
		};

		// ReSharper disable CppInconsistentNaming
		constexpr auto _Month = std::string_view(__DATE__, 3);
		constexpr auto _Day   = std::string_view(_Month._Unchecked_end( ) + 1, 2);
		constexpr auto _Year  = std::string_view(_Day._Unchecked_end( ) + 1, 4);
		// ReSharper restore CppInconsistentNaming

		auto month = [&]( )-> std::string_view
		{
			switch (_Month[0])
			{
				case 'J': // Jan, Jun or Jul
					return _Month[1] == 'a' ? "01" : _Month[2] == 'n' ? "06" : "07";
				case 'F': // Feb
					return "02";
				case 'M': // Mar or May
					return _Month[2] == 'r' ? "03" : "05";
				case 'A': // Apr or Aug
					return _Month[2] == 'p' ? "04" : "08";
				case 'S': // Sep
					return "09";
				case 'O': // Oct
					return "10";
				case 'N': // Nov
					return "11";
				case 'D': // Dec
					return "12";
			}
			throw;
		}( );
		auto day = std::string(_Day);
		if (!std::isdigit(day[0])) //01 02 etc
			day[0] = '0';
		auto year = _Year;

		title_append(_STRINGIZE(VS_SolutionName), ' ', day, '.', month, '.', year);
#ifdef _DEBUG
		title_append(" | ", "DEBUG", " | ", __TIME__);
#endif

#ifdef CHEAT_GUI_TEST
		title_append(" | ", "GUI TEST");
#endif

		//----------

		// ReSharper disable once CppUseStructuredBinding
		auto& w = menu_window;
		w.title.set_label(std::move(title_str));
		w.title.set_font(ImGui::GetDefaultFont( ) /*[]
		{
			auto font_cfg        = gui::fonts_builder_proxy::default_font_config( );
			font_cfg->SizePixels = 25;
			return gui::imgui_context::get_ptr( )->fonts( ).add_font_from_ttf_file(R"(C:\Windows\Fonts\verdana.TTF)", std::move(font_cfg));
		}( )*/);
		w.show_anim.set_start(0);
		auto& global_alpha = ImGui::GetStyle( ).Alpha;
		w.show_anim.set_end(/*1*/global_alpha);
		w.show_anim.set_duration(250ms);
#if CHEAT_GUI_TEST || _DEBUG
		w.show_anim.restart(true);
		w.set(true);
#else
		w.set(false);
#endif

		auto& show_anim_target = w.show_anim.set_target(nstd::smooth_object_base::target_external_tag{});
		show_anim_target.write_value(global_alpha);
		w.flags |= ImGuiWindowFlags_AlwaysAutoResize;

		//----------
	}
};

menu_impl::menu_impl( )
{
	impl_ = std::make_unique<impl>( );
	this->add_dependency(imgui_context::get( ));
	this->add_dependency(features::aimbot::get( ));
}

menu_impl::~menu_impl( ) = default;

bool menu_impl::render( )
{
	const auto end = impl_->menu_window( );
	if (!end)
		return false;

	ImGui::Text("hello");
	ImGui::Text("hello1");
	ImGui::Text("hello2");
	ImGui::Text("hello3");
	ImGui::Text("hello4");
	ImGui::Text("hello5");

	features::aimbot::get( )->render( );

#if 0
	if (this->begin(impl_->menu_title, ImGuiWindowFlags_AlwaysAutoResize))
	{
		impl_->tabs_pages.render();
		//-
	}
	this->end();
#endif

	return true;
}

bool menu_impl::toggle(UINT msg, WPARAM wparam)
{
	if (wparam != impl_->hotkey)
		return false;

	if (msg == WM_KEYDOWN)
	{
		//hide press from app
		return true;
	}
	if (msg == WM_KEYUP)
	{
		impl_->menu_window.toggle( );
		return true;
	}

	return false;
}

bool menu_impl::visible( ) const
{
	return impl_->menu_window.visible( );
}

bool menu_impl::updating( ) const
{
	return impl_->menu_window.updating( );
}

#if 0
class unused_page final : public renderable, public string_wrapper_base
{
	unused_page(string_wrapper&& other)
		: string_wrapper_base(std::move(other))
	{
	}

public:
	void render() override
	{
#if CHEAT_GUI_HAS_IMGUI_STRV
		ImGui::TextUnformatted(*this);
#else
		auto&& mb = this->multibyte();
		ImGui::TextUnformatted(mb._Unchecked_begin(), mb._Unchecked_end());
#endif
	}

	static unused_page* get_ptr()
	{
		static size_t counter = 0;
		return new unused_page("unused page " + std::to_string(counter++));
	}
};
#endif

auto menu_impl::load_impl( ) noexcept -> load_result
{
#if 0
	renderer_.add_page([]
		{
			auto  rage_abstract = abstract_page();
			auto& rage = *rage_abstract.init<horizontal_pages_renderer>("rage");

			using namespace features;
			rage.add_page(aimbot::get_ptr());
			rage.add_page(anti_aim::get_ptr());

			return rage_abstract;
		}());
	renderer_.add_page({ "settings", settings::get_ptr_shared() });

#if defined(_DEBUG)
	renderer_.add_page([]
		{
			auto  debug_abstract = abstract_page();
			auto& debug = *debug_abstract.init<vertical_pages_renderer>("DEBUG");

			debug.add_page([]
				{
					auto  debug_hooks_abstract = abstract_page();
					auto& debug_hooks = *debug_hooks_abstract.init<vertical_pages_renderer>("hooks");

					const auto add_if_hookded = [&]<typename Tstr, typename Tptr>(Tstr && name, Tptr && ptr)
					{
						basic_service* ptr_raw = ptr.get();

						switch (ptr_raw->state().value())
						{
						case service_state::waiting:
						case service_state::loading:
						case service_state::loaded:
							break;
						default:
							return;
						}

						if (const auto skipped = dynamic_cast<service_maybe_skipped*>(ptr_raw); skipped != nullptr && skipped->always_skipped())
							return;

						debug_hooks.add_page({ std::forward<Tstr>(name), std::forward<Tptr>(ptr) });
					};
					using namespace hooks;
					add_if_hookded("window proc", winapi::wndproc::get_ptr_shared());
					add_if_hookded("should skip animation frame", c_base_animating::should_skip_animation_frame::get_ptr_shared());

					return debug_hooks_abstract;
				}());
			debug.add_page(unused_page::get_ptr());
			debug.add_page(unused_page::get_ptr());

			return debug_abstract;
		}());
#endif
	renderer_.init();
#endif

	impl_->init_window( );
	impl_->init_pages( );

	CHEAT_SERVICE_LOADED
}

CHEAT_SERVICE_REGISTER(menu);

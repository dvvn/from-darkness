#include "menu.h"

#include "imgui context.h"

#include "cheat/core/services loader.h"
#include "cheat/features/aimbot.h"
//#include "cheat/features/anti aim.h"
//#include "cheat/hooks/c_baseanimating/should skip animation_ frame.h"
//#include "cheat/hooks/winapi/wndproc.h"

#include "widgets/tab_bar_with_pages.h"
#include "tools/cached_text.h"

#include <nstd/runtime assert.h>

#include <imgui.h>
#include <imgui_internal.h>

#include <Windows.h>

#include <algorithm>
#include <functional>
#include <random>
#include <sstream>

using namespace cheat;
using namespace gui;
using namespace objects;
using namespace tools;
using namespace widgets;

struct menu::impl
{
	WPARAM hotkey = VK_HOME;

	imgui_string menu_title;
	tab_bar_with_pages tabs_pages;

	void init_pages()
	{
		tabs_pages.make_vertical( );
		tabs_pages.make_size_static( );

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
				const auto selected_before = source->get_selected( );
				if (selected_before == self)
					return;

				selected_before->deselect( );
				self->select( );
			};

			return make_callback_info(std::move(fn), false);
		};

		constexpr auto add_item_set_callbacks = [&] <typename C, size_t S, typename T>(tab_bar_with_pages& tab_bar, const C (&name)[S], const std::shared_ptr<T>& data)-> T&
		{
			using namespace std::chrono_literals;

			auto item = std::make_unique<tab_bar_item>( );

			item->set_font(ImGui::GetDefaultFont( ));
			item->set_label((name));
			auto anim = std::make_unique<animation_property_linear<ImVec4>>( );
			anim->set_duration(400ms);
			item->set_background_color_modifier(std::move(anim));
			item->add_pressed_callback(make_pressed_callback(std::addressof(tab_bar), item.get( )), two_way_callback::WAY_TRUE);

			tab_bar.add_item(std::move(item), data);

			return *data;
		};

		auto& rage_tab = add_item_set_callbacks(tabs_pages, "rage", std::make_shared<tab_bar_with_pages>( ));

		rage_tab.make_horisontal( );
		rage_tab.make_size_auto( );
		add_item_set_callbacks(rage_tab, "aimbot", features::aimbot::get_ptr_shared( ));
		add_item_set_callbacks(rage_tab, "aimbot1", features::aimbot::get_ptr_shared( ));
		add_item_set_callbacks(rage_tab, "aimbot2", features::aimbot::get_ptr_shared( ));
	}

	impl()
	{
		const auto init_title = [&]
		{
			constexpr auto iso_date = []
			{
				// ReSharper disable CppVariableCanBeMadeConstexpr

				const uint32_t compile_year  = (__DATE__[7] - '0') * 1000 + (__DATE__[8] - '0') * 100 + (__DATE__[9] - '0') * 10 + (__DATE__[10] - '0');
				const uint32_t compile_month = []
				{
					switch (__DATE__[0])
					{
						case 'J': // Jan, Jun or Jul
							return __DATE__[1] == 'a' ? 1 : __DATE__[2] == 'n' ? 6 : 7;
						case 'F': // Feb
							return 2;
						case 'M': // Mar or May
							return __DATE__[2] == 'r' ? 3 : 5;
						case 'A': // Apr or Aug
							return __DATE__[2] == 'p' ? 4 : 8;
						case 'S': // Sep
							return 9;
						case 'O': // Oct
							return 10;
						case 'N': // Nov
							return 11;
						case 'D': // Dec
							return 12;
					}
					throw;
				}( );

				// ReSharper disable once CppUnreachableCode
				const uint32_t compile_day = __DATE__[4] == ' ' ? __DATE__[5] - '0' : (__DATE__[4] - '0') * 10 + (__DATE__[5] - '0');

				const char ret[] = {

						compile_year / 1000 + '0', compile_year % 1000 / 100 + '0', compile_year % 100 / 10 + '0', compile_year % 10 + '0', '.', compile_month / 10 + '0'
					  , compile_month % 10 + '0', '.', compile_day / 10 + '0', compile_day % 10 + '0'
					  , '\0'
				};

				// ReSharper restore CppVariableCanBeMadeConstexpr
				return std::string_view(ret, std::size(ret) - 1);
			};

			auto name = std::ostringstream( );

			const auto append_name = [&]<class T>(T&& text, bool delim = true)
			{
				if (delim)
					name << " | ";
				name << text;
			};

			append_name(_STRINGIZE(VS_SolutionName), false);
			append_name(iso_date( ));
#ifdef _DEBUG
			append_name("DEBUG");
			append_name(__TIME__);
#endif

#ifdef CHEAT_GUI_TEST
			append_name("GUI TEST");
#endif

			menu_title = std::move(name).str( );
		};

		//-------

		init_title( );
	}
};

menu::menu()
{
	impl_ = std::make_unique<impl>( );
	this->wait_for_service<imgui_context>( );
}

menu::~menu() = default;

void menu::render()
{
	if (this->begin(impl_->menu_title, ImGuiWindowFlags_AlwaysAutoResize))
	{
		impl_->tabs_pages.render( );
		//-
	}
	this->end( );
}

bool menu::toggle(UINT msg, WPARAM wparam)
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
		window::toggle( );
		return true;
	}

	return false;
}

#if 0
class unused_page final : public renderable, public string_wrapper_base
{
	unused_page(string_wrapper&& other)
		: string_wrapper_base(std::move(other))
	{
	}

public:
	void render( ) override
	{
#if CHEAT_GUI_HAS_IMGUI_STRV
		ImGui::TextUnformatted(*this);
#else
		auto&& mb = this->multibyte( );
		ImGui::TextUnformatted(mb._Unchecked_begin( ), mb._Unchecked_end( ));
#endif
	}

	static unused_page* get_ptr( )
	{
		static size_t counter = 0;
		return new unused_page("unused page " + std::to_string(counter++));
	}
};
#endif

service_base::load_result menu::load_impl()
{
#if 0
	renderer_.add_page([]
	{
		auto  rage_abstract = abstract_page( );
		auto& rage = *rage_abstract.init<horizontal_pages_renderer>("rage");

		using namespace features;
		rage.add_page(aimbot::get_ptr( ));
		rage.add_page(anti_aim::get_ptr( ));

		return rage_abstract;
	}());
	renderer_.add_page({"settings", settings::get_ptr_shared( )});

#if defined(_DEBUG)
	renderer_.add_page([]
	{
		auto  debug_abstract = abstract_page( );
		auto& debug = *debug_abstract.init<vertical_pages_renderer>("DEBUG");

		debug.add_page([]
		{
			auto  debug_hooks_abstract = abstract_page( );
			auto& debug_hooks = *debug_hooks_abstract.init<vertical_pages_renderer>("hooks");

			const auto add_if_hookded = [&]<typename Tstr, typename Tptr>(Tstr && name, Tptr && ptr)
			{
				service_base* ptr_raw = ptr.get( );

				switch(ptr_raw->state( ).value( ))
				{
				case service_state::waiting:
				case service_state::loading:
				case service_state::loaded:
					break;
				default:
					return;
				}

				if(const auto skipped = dynamic_cast<service_maybe_skipped*>(ptr_raw); skipped != nullptr && skipped->always_skipped( ))
					return;

				debug_hooks.add_page({std::forward<Tstr>(name), std::forward<Tptr>(ptr)});
			};
			using namespace hooks;
			add_if_hookded("window proc", winapi::wndproc::get_ptr_shared( ));
			add_if_hookded("should skip animation frame", c_base_animating::should_skip_animation_frame::get_ptr_shared( ));

			return debug_hooks_abstract;
		}());
		debug.add_page(unused_page::get_ptr( ));
		debug.add_page(unused_page::get_ptr( ));

		return debug_abstract;
	}());
#endif
	renderer_.init( );
#endif

	impl_->init_pages( );

	co_return service_state::loaded;
}

CHEAT_REGISTER_SERVICE(menu);

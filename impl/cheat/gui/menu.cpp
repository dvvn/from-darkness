﻿#include "menu.h"

#include "imgui context.h"

#include "cheat/core/services loader.h"
#include "cheat/features/aimbot.h"
//#include "cheat/features/anti aim.h"
//#include "cheat/hooks/c_baseanimating/should skip animation frame.h"
//#include "cheat/hooks/winapi/wndproc.h"

#include "tools/string wrapper.h"

#include "widgets/tab_bar_with_pages.h"

#include <nstd/runtime assert.h>

#include <Windows.h>

#include <algorithm>
#include <imgui.h>
#include <imgui_internal.h>
#include <random>
#include <sstream>
#include <functional>

using namespace cheat;
using namespace gui;
using namespace objects;
using namespace tools;
using namespace widgets;

struct menu::impl
{
	WPARAM hotkey = VK_HOME;

	string_wrapper     menu_title;
	tab_bar_with_pages tabs_pages;

	void init_pages( )
	{
		tabs_pages.make_vertical( );
		tabs_pages.make_size_static( );

		constexpr auto make_pressed_callback = [](tab_bar* source)
		{
			return [=](const tab_bar_item::callback_data& data, [[maybe_unused]] const tab_bar_item::callback_state& state)
			{
				const auto selected_before = source->get_selected( );
				const auto current         = static_cast<tab_bar_item*>(data.caller);
				if (selected_before == current)
					return;
				selected_before->deselect( );
				current->select( );
			};
		};
		constexpr auto add_item_set_callbacks =
			[&]
			<typename C, size_t S, typename T>(tab_bar_with_pages& tab_bar, const C (&name)[S], const std::shared_ptr<T>& data)-> T&
		{
			auto& item = tab_bar.add_item(name, data);

			item.add_pressed_callback(make_pressed_callback(std::addressof(tab_bar)), false);
			return *data;
		};

		auto& rage_tab = add_item_set_callbacks(tabs_pages, "rage", std::make_shared<tab_bar_with_pages>( ));

		rage_tab.make_horisontal( );
		rage_tab.make_size_auto( );
		add_item_set_callbacks(rage_tab, "aimbot", features::aimbot::get_ptr_shared(true));
		add_item_set_callbacks(rage_tab, "aimbot1", features::aimbot::get_ptr_shared( ));
		add_item_set_callbacks(rage_tab, "aimbot2", features::aimbot::get_ptr_shared( ));
	}

	impl( )
	{
		const auto init_title = [&]
		{
			constexpr auto iso_date = []
			{
				// ReSharper disable once CppVariableCanBeMadeConstexpr
				const uint32_t compile_year = (__DATE__[7] - '0') * 1000 + (__DATE__[8] - '0') * 100 + (__DATE__[9] - '0') * 10 + (__DATE__[10] - '0');
				// ReSharper disable once CppVariableCanBeMadeConstexpr
				const uint32_t compile_month = []
				{
					switch (__DATE__[0])
					{
						case 'J': // Jan, Jun or Jul
							return (__DATE__[1] == 'a' ? 1 : __DATE__[2] == 'n' ? 6 : 7);
						case 'F': // Feb
							return 2;
						case 'M': // Mar or May
							return (__DATE__[2] == 'r' ? 3 : 5);
						case 'A': // Apr or Aug
							return (__DATE__[2] == 'p' ? 4 : 8);
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

				// ReSharper disable once CppVariableCanBeMadeConstexpr
				// ReSharper disable once CppUnreachableCode
				const uint32_t compile_day = __DATE__[4] == ' ' ? __DATE__[5] - '0' : (__DATE__[4] - '0') * 10 + (__DATE__[5] - '0');

				// ReSharper disable once CppVariableCanBeMadeConstexpr
				const char ret[] = {

					compile_year / 1000 + '0', compile_year % 1000 / 100 + '0', compile_year % 100 / 10 + '0', compile_year % 10 + '0',
					'.',
					compile_month / 10 + '0', compile_month % 10 + '0',
					'.',
					compile_day / 10 + '0', compile_day % 10 + '0',
					'\0'
				};
				return std::string_view(ret, std::size(ret) - 1);
			};

			std::ostringstream name;
			name
				<< _STRINGIZE(VS_SolutionName)
				<< " | "
				<< iso_date( )
#ifdef _DEBUG
				<< " | "
				<< __TIME__
#endif

#ifdef CHEAT_GUI_TEST
				<< " (gui test)"
#endif

#ifdef _DEBUG
				<< " DEBUG MODE"
#endif
				;

			menu_title = std::move(name).str( );
		};

		//-------

		init_title( );
	}
};

menu::menu( )
{
	impl_ = std::make_unique<impl>( );
	this->wait_for_service<imgui_context>( );
}

menu::~menu( ) = default;

void menu::render( )
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

service_base::load_result menu::load_impl( )
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

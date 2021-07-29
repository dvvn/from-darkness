#include "menu.h"

#include "cheat/gui/tools/push style var.h"

#include "cheat/features/aimbot.h"
#include "cheat/features/anti aim.h"
#include "cheat/hooks/c_baseanimating/should skip animation frame.h"

using namespace cheat;
using namespace utl;
using namespace gui;
using namespace objects;
using namespace tools;
using namespace widgets;

menu::menu( )
{
	constexpr auto iso_date = []( )-> string_view
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
		return string_view(ret, std::size(ret) - 1);
	};

	string name = CHEAT_NAME;
	name += " | ";
	name += iso_date( );
#ifdef _DEBUG
	name += _CONCAT(" | ", __TIME__);
#endif

#ifdef CHEAT_GUI_TEST
	name += " (gui test)";
#endif

#ifdef _DEBUG
	name += " DEBUG MODE";
#endif

	menu_title__ = move(name);
}

void menu::render( )
{
	if (this->begin(menu_title__, ImGuiWindowFlags_AlwaysAutoResize))
	{
		renderer__.render( );
	}
	this->end( );
}

bool menu::toggle(UINT msg, WPARAM wparam)
{
	if (wparam != hotkey__)
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

class unused_page final: public empty_page, public string_wrapper_base
{
	unused_page(string_wrapper&& other) : string_wrapper_base(move(other))
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
		return new unused_page("unused page " + to_string(counter++));
	}
};

bool menu::Do_load( )
{
	renderer__.add_page([]
	{
		auto rage_abstract = abstract_page( );
		auto& rage = *rage_abstract.init<horizontal_pages_renderer>("rage");

		using namespace features;
		rage.add_page(aimbot::get_ptr( ));
		rage.add_page(anti_aim::get_ptr( ));

		return rage_abstract;
	}( ));
	renderer__.add_page({"settings", settings::get_ptr( )});

#if defined(_DEBUG) && false
	renderer__.add_page([]
	{
		auto debug_abstract = abstract_page( );
		auto& debug = *debug_abstract.init<vertical_pages_renderer>("DEBUG");

		debug.add_page([]
		{
			auto debug_hooks_abstract = abstract_page( );
			auto& debug_hooks = *debug_hooks_abstract.init<horizontal_pages_renderer>("hooks");

			const auto add_if_hookded = [&]<typename Tstr,typename Tptr>(Tstr&& name, Tptr* ptr)
			{
				if (!ptr->hooked( ))
					return;
				debug_hooks.add_page({name, ptr});
			};
			using namespace hooks;
			add_if_hookded("window proc", input::wndproc::get_ptr( ));
			add_if_hookded("should skip animation frame", c_base_animating::should_skip_animation_frame::get_ptr( ));

			return debug_hooks_abstract;
		}( ));
		debug.add_page(unused_page::get_ptr( ));
		debug.add_page(unused_page::get_ptr( ));

		return debug_abstract;
	}( ));
#endif
	renderer__.init( );

	return true;
}

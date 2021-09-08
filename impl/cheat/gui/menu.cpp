#include "menu.h"
#include "imgui context.h"

#include "cheat/features/aimbot.h"
//#include "cheat/features/anti aim.h"
//#include "cheat/hooks/c_baseanimating/should skip animation frame.h"
//#include "cheat/hooks/winapi/wndproc.h"

#include "widgets/tab_bar_with_pages.h"

#include <algorithm>
#include <imgui.h>
#include <imgui_internal.h>
#include <random>

#include <Windows.h>

using namespace cheat;
using namespace gui;
using namespace objects;
using namespace tools;
using namespace widgets;

static constexpr std::string_view _Iso_date( )
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

//todo: move outside
namespace cheat::gui::widgets
{
	class text: public renderable
	{
	public:
		void render( ) override
		{
			ImGui::Text(text_);
		}

		void set(string_wrapper&& text)
		{
			text_ = std::move(text);
		}

	private:
		string_wrapper text_;
	};

	template <typename ...Ts>
	class multi_widget final: public renderable, public std::tuple<Ts...>
	{
		template <size_t ...I>
		void render_impl(std::index_sequence<I...>)
		{
			(static_cast<renderable&>(std::get<I>(*this)).render( ), ...);
		}

	public:
		void render( ) override
		{
			render_impl(std::index_sequence_for<Ts...>( ));
		}
	};
};

struct menu::impl
{
	string_wrapper menu_title;
	WPARAM         hotkey = VK_HOME;

	tab_bar_with_pages pages;

	impl( )
	{
		const auto init_title = [&]
		{
			std::string name = /*CHEAT_NAME*/_STRINGIZE(VS_SolutionName);
			name += " | ";
			name += _Iso_date( );
#ifdef _DEBUG
			name += _CONCAT(" | ", __TIME__);
#endif

#ifdef CHEAT_GUI_TEST
			name += " (gui test)";
#endif

#ifdef _DEBUG
			name += " DEBUG MODE";
#endif

			menu_title = std::move(name);
		};

		const auto init_tabs = [&]
		{
			pages->make_vertical( );
			pages->make_size_static( );

			auto& rage_tab = *pages.add_item<tab_bar_with_pages>("rage"_shl);
			rage_tab.add_item(features::aimbot::get_ptr_shared(true));

			rage_tab->make_horisontal( );
			rage_tab->make_size_auto( );

			//---

			auto& tab2 = *pages.add_item<tab_bar_with_pages>("test tab"_shl);
			tab2->make_horisontal( );
			tab2->make_size_auto( );

			std::random_device rd;
			std::mt19937       gen{rd( )};

			auto       names     = std::array{"text"_shl, "text1"_shl, "1"_shl, "2"_shl, "long text"_shl, "longest text"_shl,};
			const auto add_names = [&](tab_bar* where)
			{
				std::ranges::shuffle(names, gen);
				for (auto& n: names)
					where->add_tab(n);
			};

			auto& tab2_0all = *tab2.add_item<multi_widget<text, tab_bar>>("zero"_shl);
			std::get<text>(tab2_0all).set("hello from multi widget");
			auto tab2_0 = std::addressof(std::get<tab_bar>(tab2_0all));
			add_names(tab2_0);
			tab2_0->make_horisontal( );
			tab2_0->make_size_static( );
			auto tab2_1 = tab2.add_item<tab_bar>("one"_shl);
			add_names(tab2_1);
			tab2_1->make_horisontal( );
			tab2_1->make_size_auto( );
			auto tab2_2 = tab2.add_item<tab_bar>("two"_shl);
			add_names(tab2_2);
			tab2_2->make_vertical( );
			tab2_2->make_size_auto( );
			auto tab2_3 = tab2.add_item<tab_bar>("three"_shl);
			add_names(tab2_3);
			tab2_3->make_vertical( );
			tab2_3->make_size_static( );
		};

		init_title( );
		init_tabs( );
	}
};

menu::menu( )
{
}

menu::~menu( ) = default;

void menu::render( )
{
	if (this->begin(impl_->menu_title, ImGuiWindowFlags_AlwaysAutoResize))
	{
		impl_->pages.render( );
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
class unused_page final: public renderable, public string_wrapper_base
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
		auto& rage          = *rage_abstract.init<horizontal_pages_renderer>("rage");

		using namespace features;
		rage.add_page(aimbot::get_ptr( ));
		rage.add_page(anti_aim::get_ptr( ));

		return rage_abstract;
	}( ));
	renderer_.add_page({"settings", settings::get_ptr_shared( )});

#if defined(_DEBUG)
	renderer_.add_page([]
	{
		auto  debug_abstract = abstract_page( );
		auto& debug          = *debug_abstract.init<vertical_pages_renderer>("DEBUG");

		debug.add_page([]
		{
			auto  debug_hooks_abstract = abstract_page( );
			auto& debug_hooks          = *debug_hooks_abstract.init<vertical_pages_renderer>("hooks");

			const auto add_if_hookded = [&]<typename Tstr,typename Tptr>(Tstr&& name, Tptr&& ptr)
			{
				service_base* ptr_raw = ptr.get( );

				switch (ptr_raw->state( ).value( ))
				{
					case service_state::waiting:
					case service_state::loading:
					case service_state::loaded:
						break;
					default:
						return;
				}

				if (const auto skipped = dynamic_cast<service_sometimes_skipped*>(ptr_raw); skipped != nullptr && skipped->always_skipped( ))
					return;

				debug_hooks.add_page({std::forward<Tstr>(name), std::forward<Tptr>(ptr)});
			};
			using namespace hooks;
			add_if_hookded("window proc", winapi::wndproc::get_ptr_shared( ));
			add_if_hookded("should skip animation frame", c_base_animating::should_skip_animation_frame::get_ptr_shared( ));

			return debug_hooks_abstract;
		}( ));
		debug.add_page(unused_page::get_ptr( ));
		debug.add_page(unused_page::get_ptr( ));

		return debug_abstract;
	}( ));
#endif
	renderer_.init( );
#endif

	impl_ = std::make_unique<impl>( );

	co_return service_state::loaded;
}

#include "tab_bar_with_pages.h"

#include "nstd/runtime assert.h"

#include <imgui.h>

#include <vector>

using namespace cheat::gui;
using namespace widgets;
using namespace objects;
using namespace tools;

struct tab_bar_with_pages::impl
{
	tab_bar                        bar;
	std::vector<renderable_shared> wnds;
};

tab_bar_with_pages::tab_bar_with_pages( )
{
	impl_ = std::make_unique<impl>( );
}

tab_bar_with_pages::~tab_bar_with_pages( ) = default;

void tab_bar_with_pages::render( )
{
	auto& [_Bar, _Wnds] = *impl_;
	runtime_assert(!_Wnds.empty());

	ImGui::BeginGroup( );
	{
		_Bar.render( );

		auto& selected = [&]( )-> decltype(auto)
		{
			const auto target = _Bar.get_selected( ); //*_Wnds[_Bar.get_selected_index( )]
			const auto first  = _Bar.begin( );

			const auto index = std::distance(first, target);
			return *_Wnds[index];
		}( );

		const auto render_selected_horisontal = [&]
		{
			selected.render( );
		};
		const auto render_selected_verical = [&]
		{
			ImGui::SameLine( );
			selected.render( );
			//ImGui::SameLine( );
		};

		_Bar.is_vertical( ) ? render_selected_verical( ) : render_selected_horisontal( );
	}
	ImGui::EndGroup( );
}

// ReSharper disable once CppMemberFunctionMayBeConst
tab_bar_item& tab_bar_with_pages::add_item(string_wrapper&& bar_name, const renderable_shared& data)
{
	auto& [bar, wnds] = *impl_;

	wnds.push_back(data);
	return bar.add_tab(std::move(bar_name));
}

tab_bar* tab_bar_with_pages::operator->( ) const
{
	return std::addressof(impl_->bar);
}

// ReSharper disable once CppMemberFunctionMayBeConst
renderable* tab_bar_with_pages::get_item(perfect_string&& title)
{
	auto& [bar, wnds] = *impl_;

	const auto target = bar.find_tab(std::move(title));
	const auto first  = bar.begin( );

	const auto index = std::distance(first, target);
	return wnds[index].get( );
}

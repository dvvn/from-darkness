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

		auto& selected = *_Wnds[_Bar.get_selected_index( )];

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
void tab_bar_with_pages::add_item(const shared_label& bar_name, const renderable_shared& data)
{
	auto& [bar, wnds] = *impl_;

	bar.add_tab(bar_name);
	wnds.push_back(data);
}

tab_bar* tab_bar_with_pages::operator->( ) const
{
	return std::addressof(impl_->bar);
}

// ReSharper disable once CppMemberFunctionMayBeConst
renderable* tab_bar_with_pages::get_item(tools::perfect_string&& title)
{
	const auto index = impl_->bar.get_index(std::move(title));
	return impl_->wnds[index].get( );
}

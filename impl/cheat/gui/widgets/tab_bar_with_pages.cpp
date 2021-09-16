#include "tab_bar_with_pages.h"

#include "nstd/runtime assert.h"

#include <imgui.h>

#include <vector>

using namespace cheat::gui;
using namespace widgets;
using namespace objects;
using namespace tools;

struct tab_bar_with_pages::pages: std::vector<renderable_shared>
{
};

tab_bar_with_pages::tab_bar_with_pages( )
{
	pages_ = std::make_unique<pages>( );
}

tab_bar_with_pages::~tab_bar_with_pages( ) = default;

void tab_bar_with_pages::render( )
{
	runtime_assert(!pages_->empty());

	ImGui::BeginGroup( );
	{
		tab_bar::render( );

		auto& selected = [&]( )-> decltype(auto)
		{
			const auto target = this->get_selected( ); //*_Wnds[this->get_selected_index( )]
			const auto index  = this->find_tab_index(target);
			return *(*pages_)[index];
		}( );

		const auto render_selected = [&]
		{
			ImGui::BeginGroup( );
			selected.render( );
			ImGui::EndGroup( );
		};

		const auto render_selected_horisontal = [&]
		{
			render_selected( );
		};
		const auto render_selected_verical = [&]
		{
			ImGui::SameLine( );
			render_selected( );
			//ImGui::SameLine( );
		};

		this->is_vertical( ) ? render_selected_verical( ) : render_selected_horisontal( );
	}
	ImGui::EndGroup( );
}

// ReSharper disable once CppMemberFunctionMayBeConst
void tab_bar_with_pages::add_item(std::unique_ptr<tab_bar_item>&& bar_item, const renderable_shared& data)
{
	this->add_tab(std::move(bar_item));
	pages_->push_back(data);
}

// ReSharper disable once CppMemberFunctionMayBeConst
renderable* tab_bar_with_pages::find_item(perfect_string&& title)
{
	const auto target = this->find_tab(std::move(title));
	const auto index  = this->find_tab_index(target);
	return (*pages_)[index].get( );
}

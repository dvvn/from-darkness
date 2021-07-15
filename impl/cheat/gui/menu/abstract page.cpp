#include "abstract page.h"

using namespace cheat;
using namespace gui;
using namespace imgui;
using namespace menu;
using namespace utl;

renderable_object* abstract_page::page( ) const
{
	return addressof(visit(overload(bind_front(&unique_ptr<renderable_object>::operator*),
									bind_front(&reference_wrapper<renderable_object>::get)), page__));
}

void abstract_page::render( )
{
	ImGui::PushID(reinterpret_cast<ImGuiID>(this));
	page( )->render( );
	ImGui::PopID( );
}

pages_storage_data::pages_storage_data(abstract_page&& page): abstract_page(move(page))
{
}

string_wrapper::value_type pages_storage_data::Name( ) const
{
	return this->name( );
}

const string_wrapper& abstract_page::name( ) const
{
	return name__.get( );
}

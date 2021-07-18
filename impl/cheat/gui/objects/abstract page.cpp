#include "abstract page.h"

using namespace cheat;
using namespace gui;
using namespace tools;
using namespace objects;
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

pages_storage_data::pages_storage_data(abstract_page&& page) : abstract_page(move(page))
{
}

string_wrapper::value_type pages_storage_data::Label( ) const
{
	return this->name( );
}

const string_wrapper& abstract_page::name( ) const
{
	return name__.get( );
}

void abstract_pages_renderer::add_page(abstract_page&& page)
{
#ifdef _DEBUG
	auto& name = page.name( );
	for (abstract_page& page_stored: *this)
	{
		if (page_stored.name( ) == name)
			BOOST_ASSERT("Duplicate detected!");
	}
#endif
	this->push_back(move(page));
}

void abstract_pages_renderer::init( )
{
#if defined(_DEBUG) && _CONTAINER_DEBUG_LEVEL <= 0
	BOOST_ASSERT(!this->empty( ));
#endif

	for (auto& p: *this)
	{
		if (const auto obj = dynamic_cast<abstract_pages_renderer*>(p.page( )))
			obj->init( );
	}

	this->front( ).select( );
}



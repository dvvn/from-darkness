#include "abstract page.h"

using namespace cheat;
using namespace gui;
using namespace tools;
using namespace objects;
using namespace utl;

renderable_object* abstract_page::page( ) const
{
	return addressof(visit(overload(bind_front(&unique_page::operator*),
									bind_front(&shared_page::operator*<renderable_object>),
									bind_front(&ref_page::get)), page__));
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
	if (const auto renderer = dynamic_cast<abstract_pages_renderer*>((page.page( ))); renderer != nullptr)
	{
		if (renderer->pages_count( ) == 0)
		{
			DebugBreak( );
			return;
		}
	}

	auto& name = page.name( );
	for (abstract_page& page_stored: pages_)
	{
		if (page_stored.name( ) == name)
			BOOST_ASSERT("Duplicate detected!");
	}
#endif
	pages_.push_back(move(page));
}

void abstract_pages_renderer::init( )
{
#if defined(_DEBUG) && _CONTAINER_DEBUG_LEVEL == 0
	BOOST_ASSERT(!this->empty( ));
#endif

	for (auto& p: pages_)
	{
		if (const auto obj = dynamic_cast<abstract_pages_renderer*>(p.page( )))
			obj->init( );
	}

	pages_.front( ).select( );
}

size_t abstract_pages_renderer::pages_count( ) const
{
	return pages_.size( );
}
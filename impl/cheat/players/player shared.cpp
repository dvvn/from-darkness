#include "players list.h"

using namespace cheat;
using namespace detail;
using namespace csgo;
using namespace utl;

void player_shared::init(C_CSPlayer* owner)
{
	BOOST_ASSERT(!*this);
	*static_cast<player::shared_type*>(this) = (utl::make_shared<player>(owner));
}

player_shared::~player_shared( )
{
#ifndef CHEAT_NETVARS_UPDATING
	if (!*this)
		return;
	BOOST_ASSERT(get()->in_use__);
	get( )->in_use__ = false;

	const auto reset_clientside_anim = [&]
	{
		__try
		{
			get( )->ent__->m_bClientSideAnimation( ) = true;
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
		}
	};
	reset_clientside_anim( );
#endif
}

player_shared::player_shared(player_shared&& other) noexcept
{
	(*this) = move(other);
}

void player_shared::operator=(player_shared&& other) noexcept
{
	*static_cast<player::shared_type*>(this) = move(other);
}

player::shared_type player_shared::share( ) const
{
	return *this;
}
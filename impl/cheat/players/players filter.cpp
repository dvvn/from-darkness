#include "players list.h"
#include "cheat/core/csgo interfaces.h"

using namespace cheat;
using namespace detail;
using namespace csgo;
using namespace utl;

static players_filter_bitflags _Get_flags_for_player(const player::shared_type& pl)
{
	if (!pl)
		return null;

	auto bflags = players_filter_bitflags( );

	const auto p = pl->owner( );
	bflags.add(p->m_iHealth( ) == 0 ? dead : alive);
	if (p->IsDormant( ))
		bflags.add(dormant);

	const auto local_team = []
	{
		return static_cast<m_iTeamNum_t>(csgo_interfaces::get( ).local_player->m_iTeamNum( ));
	};

	switch (static_cast<m_iTeamNum_t>(p->m_iTeamNum( )))
	{
		case TEAM_Spectator:
			bflags.add(spectator, enemy);
			break;
		case TEAM_Terrorist:
			bflags.add(local_team( ) == TEAM_Terrorist ? ally : enemy);
			break;
		case TEAM_CT:
			bflags.add(local_team( ) == TEAM_CT ? ally : enemy);
			break;
	}

	return bflags;
}

static bool _Player_pass_flags(const player::shared_type& p, players_filter_bitflags f)
{
	return _Get_flags_for_player(p).has(f);
}

players_filter::players_filter(const players_list_container_interface& cont, players_filter_bitflags f): flags__(f)
{
	if (f == all)
		items__ = cont;
	else
	{
		for (auto& p: cont)
		{
			if (_Player_pass_flags(p, f))
				items__.push_back(p);
		}
	}
}

players_filter::players_filter(players_list_container_interface&& cont, players_filter_bitflags f): flags__(f)
{
	if (f == all)
		items__ = move(cont);
	else
	{
		for (auto& p: cont)
		{
			if (_Player_pass_flags(p, f))
				items__.push_back(move(p));
		}
	}
}

players_filter& players_filter::add_flags(players_filter_bitflags f)
{
	if (flags__ != flags__.add(f))
	{
		auto to_remove = ranges::remove_if(items__, [f](players_list_container_interface::const_reference p)
		{
			return !_Player_pass_flags(p, f);
		});

		items__.erase(to_remove.begin( ), to_remove.end( ));
	}

	return *this;
}

players_filter players_filter::add_flags(players_filter_bitflags f) const
{
	return players_filter(items__, f);
}

players_filter_bitflags players_filter::flags( ) const
{
	return flags__;
}

size_t std::hash<players_filter>::operator()(const players_filter& val) const noexcept
{
	return hash<std::underlying_type_t<players_filter_flags>>( )(val.flags( ).convert( ));
}

bool std::equal_to<players_filter>::operator()(const players_filter& a, const players_filter& b) const noexcept
{
	return equal_to<std::underlying_type_t<players_filter_flags>>( )(a.flags( ).convert( ), b.flags( ).convert( ));
}

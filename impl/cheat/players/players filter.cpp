#include "players list.h"
#include "cheat/core/csgo interfaces.h"

using namespace cheat;
using namespace detail;
using namespace csgo;
using namespace utl;

static bool _Player_pass_flags(const player_shared& p, const players_filter_flags& f)
{
	if (!p)
		return false;

	const auto ent = p->ent;

	if (f.alive != ent->m_iHealth( ) > 0)
		return false;
	if (f.dormant != ent->IsDormant( ))
		return false;
	if (f.immune != ent->m_bGunGameImmunity( ))
		return false;

	const auto ent_team = static_cast<m_iTeamNum_t>(ent->m_iTeamNum( ));
	if (ent_team == TEAM_Unknown)
		return false;
	// ReSharper disable once CppTooWideScopeInitStatement
	const auto team_checker =
			overload([&](const players_filter_flags::team_filter& tf)
					 {
						 if (ent_team == TEAM_Spectator)
							 return false;
						 const auto local_team = static_cast<m_iTeamNum_t>(csgo_interfaces::get( ).local_player->m_iTeamNum( ));
						 return ent_team == local_team;
					 },
					 [&](const players_filter_flags::team_filter_ex& tf)
					 {
						 auto& select = static_cast<const bitflag<players_filter_flags::team_filter>&>(tf);
						 return select.has(ent_team);
					 });

	if (!visit(team_checker, f.team))
		return false;

	return true;
}

players_filter::players_filter(const players_list_container_interface& cont, const players_filter_flags& f): flags__(f)
{
	for (auto& p: cont)
	{
		if (_Player_pass_flags(p, f))
			items__.push_back(p);
	}
}

players_filter::players_filter(players_list_container_interface&& cont, const players_filter_flags& f): flags__(f)
{
	for (auto& p: cont)
	{
		if (_Player_pass_flags(p, f))
			items__.push_back(move(p));
	}
}

players_filter& players_filter::set_flags(const players_filter_flags& f)
{
	if (flags__ != f)
	{
		auto to_remove = ranges::remove_if(items__, [f](decltype(items__)::const_reference p)
		{
			return !_Player_pass_flags(p, f);
		});

		items__.erase(to_remove.begin( ), to_remove.end( ));
		flags__ = f;
	}

	return *this;
}

players_filter players_filter::set_flags(const players_filter_flags& f) const
{
	return players_filter(items__, f);
}

const players_filter_flags& players_filter::flags( ) const
{
	return flags__;
}

size_t std::hash<players_filter>::operator()(const players_filter& val) const noexcept
{
	return hash<uint64_t>( )(val.flags( ).data( ));
}

bool std::equal_to<players_filter>::operator()(const players_filter& a, const players_filter& b) const noexcept
{
	return equal_to<players_filter_flags>( )(a.flags( ), b.flags( ));
}

const uint64_t& players_filter_flags::data( ) const
{
	static_assert(sizeof(players_filter_flags) == sizeof(uint64_t));
	return *reinterpret_cast<const uint64_t*>(this);
}

bool players_filter_flags::operator==(const players_filter_flags& other) const
{
	return data( ) == other.data( );
}

bool players_filter_flags::operator!=(const players_filter_flags& other) const
{
	return !(*this == other);
}

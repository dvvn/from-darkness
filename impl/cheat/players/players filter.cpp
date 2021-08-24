#include "players list.h"
#include "cheat/core/csgo interfaces.h"

using namespace cheat;
using namespace detail;
using namespace csgo;

static bool _Player_pass_flags(const player_shared& p, const players_filter_flags& f)
{
	if (!p)
		return false;

	const auto ent = p->ent;

	const m_iTeamNum_t ent_team = ent->m_iTeamNum( );
	if (!ent_team)
		return false;

	if (f.alive != ent->IsAlive( ))
		return false;
	if (f.dormant != ent->IsDormant( ))
		return false;
	if (f.immune != ent->m_bGunGameImmunity( ))
		return false;

	// ReSharper disable once CppTooWideScopeInitStatement
	const auto team_checker
			=
			nstd::overload([&](const players_filter_flags::team_filter& tf)
						   {
							   if (ent_team.spectator( ))
								   return false;
							   using flags = players_filter_flags::team_filter;

							   const auto local_team = csgo_interfaces::get_ptr( )->local_player->m_iTeamNum( );
							   const auto enemy      = ent_team != local_team;

							   return tf.has(enemy ? flags::ENEMY : flags::ALLY);
						   },
						   [&](const players_filter_flags::team_filter_ex& tf)
						   {
							   using flags = players_filter_flags::team_filter_ex;

							   switch (ent_team.value( ))
							   {
								   case m_iTeamNum_t::SPEC: return tf.has(flags::SPEC);
								   case m_iTeamNum_t::T: return tf.has(flags::T);
								   case m_iTeamNum_t::CT: return tf.has(flags::CT);
								   default:
									   runtime_assert("unknown flag");
									   return false;
							   }
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
			items__.push_back(std::move(p));
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
	if (flags__ != f)
		return players_filter(items__, f);

	return *this;
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

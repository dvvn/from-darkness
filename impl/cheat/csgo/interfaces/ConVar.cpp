module;

#include <nstd/format.h>

#include <sstream>

module cheat.csgo.interfaces.ConVar;
import cheat.csgo.modules;
import cheat.console;
import dhooks;
import nstd.mem.address;

using namespace cheat;
using namespace csgo;

ICVar* nstd::one_instance_getter<ICVar*>::_Construct( )const
{
	return csgo_modules::vstdlib->find_game_interface("VEngineCvar");
}

template <typename T>
static void _Set_helper(ConVar* ptr, size_t index, T value)
{
	//return dhooks::_Call_function(static_cast<void(ConVar::*)(T)>(&ConVar::set), ptr, index, value);
	dhooks::invoke(&ConVar::set<T>, index, ptr, value);
}

template <typename T>
static T _Get_helper(const ConVar* ptr, size_t index)
{
	return dhooks::invoke(&ConVar::get<T>, index, ptr);
}

template < >
const char* ConVar::get( ) const
{
	return _Get_helper<const char*>(this, 11);
}

template < >
float ConVar::get( ) const
{
	return _Get_helper<float>(this, 12);
}

template < >
int ConVar::get( ) const
{
	return _Get_helper<int>(this, 13);
}

template < >
bool ConVar::get( ) const
{
	return !!this->get<int>( );
}

template < >
void ConVar::set(const char* value)
{
	_Set_helper(this, 14, value);
}

template < >
void ConVar::set(float value)
{
	_Set_helper(this, 15, value);
}

template < >
void ConVar::set(int value)
{
	_Set_helper(this, 16, value);
}

ConCommandBaseIterator ICVar::begin( )const
{
	using namespace nstd::mem;
	return basic_address(this).plus(0x30).deref<1>( ).get<ConCommandBase*>( );
}

ConCommandBaseIterator ICVar::end( )const
{
	return nullptr;
}

ConVar* ICVar::FindVar(const std::string_view name)const
{
	const auto compare = [=](const ConCommandBase& cv)
	{
		if (cv.IsCommand( ))
			return false;
		return std::memcmp(cv.m_pszName, name.data( ), name.size( )) == 0;
	};

	const auto first_cvar = this->begin( );
	const auto invalid_cvar = this->end( );

	const auto target_cvar = std::find_if(first_cvar, invalid_cvar, compare);
	if (!console::disabled( ))
	{
		std::ostringstream msg;
		msg << std::format("Cvar \"{}\"", name);
		if (target_cvar == invalid_cvar)
		{
			msg << " NOT ";
		}
		else
		{
			bool duplicate = false;
			for (auto cv = std::next(target_cvar); cv != invalid_cvar; ++cv)
			{
				if (!compare(*cv))
					continue;
				if (!duplicate)
				{
					msg << std::format("(\"{}\"", cv->m_pszName);
					duplicate = true;
				}
				else
				{
					msg << std::format(", \"{}\"", cv->m_pszName);
				}
			}
			if (duplicate)
				msg << ')';

			//we already know how long a string can be
			const auto known_end = target_cvar->m_pszName + name.size( );
			//so only look for the zero character
			const auto real_end = known_end + std::char_traits<char>::length(known_end);
			if (known_end != real_end)
				msg << std::format(" (full name: \"{}\")", std::string_view(target_cvar->m_pszName, real_end));
			msg << ' ';
		}
		msg << "found";
		console::log(msg.view( ));
	};

	return target_cvar == invalid_cvar ? nullptr : static_cast<ConVar*>(target_cvar.get( ));
}


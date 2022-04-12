module;

#include <cheat/csgo/interface.h>

#include <nstd/runtime_assert.h>

module cheat.csgo.interfaces.BaseHandle;
import cheat.csgo.interfaces.ClientEntityList;

// How many bits to use to encode an edict.
constexpr auto MAX_EDICT_BITS = 11; // # of bits needed to represent max edicts
// Max # of edicts in a level
constexpr auto MAX_EDICTS = 1 << MAX_EDICT_BITS;

// Used for networking ehandles.
constexpr auto NUM_ENT_ENTRY_BITS = MAX_EDICT_BITS + 2;
constexpr auto NUM_ENT_ENTRIES = 1 << NUM_ENT_ENTRY_BITS;
constexpr auto INVALID_EHANDLE_INDEX = 0xFFFFFFFF;

constexpr auto NUM_SERIAL_NUM_BITS = 16/*32 - NUM_ENT_ENTRY_BITS*/;
constexpr auto NUM_SERIAL_NUM_SHIFT_BITS = 32 - NUM_SERIAL_NUM_BITS;
constexpr auto ENT_ENTRY_MASK = (1 << NUM_SERIAL_NUM_BITS) - 1;

using namespace cheat::csgo;

CBaseHandle::CBaseHandle( )
{
	m_Index = INVALID_EHANDLE_INDEX;
}

CBaseHandle::CBaseHandle(const CBaseHandle& other)
{
	m_Index = other.m_Index;
}

CBaseHandle::CBaseHandle(IHandleEntity* pHandleObj)
{
	if (!pHandleObj)
	{
		m_Index = INVALID_EHANDLE_INDEX;
	}
	else
	{
		*this = pHandleObj->GetRefEHandle( );
	}
}

CBaseHandle::CBaseHandle(int iEntry, int iSerialNumber)
{
	runtime_assert(iEntry >= 0 && (iEntry & ENT_ENTRY_MASK) == iEntry);
	runtime_assert(iSerialNumber >= 0 && iSerialNumber < (1 << NUM_SERIAL_NUM_BITS));
	m_Index = iEntry | (iSerialNumber << /*NUM_ENT_ENTRY_BITS*/NUM_SERIAL_NUM_SHIFT_BITS);
}

// Even if this returns true, Get() still can return return a non-null value.
// This just tells if the handle has been initted with any values.
bool CBaseHandle::IsValid( ) const
{
	return m_Index != INVALID_EHANDLE_INDEX;
}

int32_t CBaseHandle::GetEntryIndex( ) const
{
	if (!IsValid( ))
		return NUM_ENT_ENTRIES - 1;
	return m_Index & ENT_ENTRY_MASK;
}

int32_t CBaseHandle::GetSerialNumber( ) const
{
	return m_Index >> /*NUM_ENT_ENTRY_BITS*/NUM_SERIAL_NUM_SHIFT_BITS;
}

int32_t CBaseHandle::ToInt( ) const
{
	return (int32_t)m_Index;
}

//unable to import interfaces module here

IHandleEntity* CBaseHandle::Get( ) const
{
	/*if (!IsValid( ))
		return 0;*/
	return IClientEntityList::get( ).GetClientEntityFromHandle(*this);
}

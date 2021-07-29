#include "BaseHandle.hpp"

#include "IClientEntityList.hpp"

#include "cheat/core/csgo interfaces.h"

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
	BOOST_ASSERT(iEntry >= 0 && (iEntry & ENT_ENTRY_MASK) == iEntry);
	BOOST_ASSERT(iSerialNumber >= 0 && iSerialNumber < (1 << NUM_SERIAL_NUM_BITS));
	m_Index = iEntry | (iSerialNumber << /*NUM_ENT_ENTRY_BITS*/NUM_SERIAL_NUM_SHIFT_BITS);
}

bool CBaseHandle::IsValid( ) const
{
	return m_Index != INVALID_EHANDLE_INDEX;
}

int CBaseHandle::GetEntryIndex( ) const
{
	if (!IsValid( ))
		return NUM_ENT_ENTRIES - 1;
	return m_Index & ENT_ENTRY_MASK;
}

int CBaseHandle::GetSerialNumber( ) const
{
	return m_Index >> /*NUM_ENT_ENTRY_BITS*/NUM_SERIAL_NUM_SHIFT_BITS;
}

int CBaseHandle::ToInt( ) const
{
	return (int)m_Index;
}

IHandleEntity* CBaseHandle::Get( ) const
{
	/*if (!IsValid( ))
		return 0;*/
	return csgo_interfaces::get_ptr( )->entity_list->GetClientEntityFromHandle(*this);
}

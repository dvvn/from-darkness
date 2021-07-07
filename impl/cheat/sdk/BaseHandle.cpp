#include "BaseHandle.hpp"

using namespace cheat::csgo;

CBaseHandle::CBaseHandle( )
{
	m_Index = INVALID_EHANDLE_INDEX;
}

CBaseHandle::CBaseHandle(const CBaseHandle& other)
{
	m_Index = other.m_Index;
}

CBaseHandle::CBaseHandle(unsigned long value)
{
	m_Index = value;
}

CBaseHandle::CBaseHandle(int iEntry, int iSerialNumber)
{
	Init(iEntry, iSerialNumber);
}

auto CBaseHandle::Init(int iEntry, int iSerialNumber) -> void
{
	m_Index = iEntry | (iSerialNumber << NUM_ENT_ENTRY_BITS);
}

auto CBaseHandle::Term( ) -> void
{
	m_Index = INVALID_EHANDLE_INDEX;
}

auto CBaseHandle::IsValid( ) const -> bool
{
	return m_Index != INVALID_EHANDLE_INDEX;
}

auto CBaseHandle::GetEntryIndex( ) const -> int
{
	return m_Index & ENT_ENTRY_MASK;
}

auto CBaseHandle::GetSerialNumber( ) const -> int
{
	return m_Index >> NUM_ENT_ENTRY_BITS;
}

auto CBaseHandle::ToInt( ) const -> int
{
	return (int)m_Index;
}

auto CBaseHandle::operator !=(const CBaseHandle& other) const -> bool
{
	return m_Index != other.m_Index;
}

auto CBaseHandle::operator ==(const CBaseHandle& other) const -> bool
{
	return m_Index == other.m_Index;
}

auto CBaseHandle::operator ==(const IHandleEntity* pEnt) const -> bool
{
	return Get( ) == pEnt;
}

auto CBaseHandle::operator !=(const IHandleEntity* pEnt) const -> bool
{
	return Get( ) != pEnt;
}

auto CBaseHandle::operator <(const CBaseHandle& other) const -> bool
{
	return m_Index<other.m_Index;
}

auto CBaseHandle::operator <(const IHandleEntity* pEntity) const -> bool
{
	unsigned long otherIndex = (pEntity) ? pEntity->GetRefEHandle( ).m_Index : INVALID_EHANDLE_INDEX;
	return m_Index < otherIndex;
}

auto CBaseHandle::operator=(const IHandleEntity* pEntity) -> const CBaseHandle&
{
	return Set(pEntity);
}

auto CBaseHandle::Set(const IHandleEntity* pEntity) -> const CBaseHandle&
{
	if (pEntity)
	{
		*this = pEntity->GetRefEHandle( );
	}
	else
	{
		m_Index = INVALID_EHANDLE_INDEX;
	}

	return *this;
}

auto CBaseHandle::Get( ) const -> IHandleEntity*
{
	(void)this;
	//return g_EntityList->GetClientEntityFromHandle(*this);
	BOOST_ASSERT(0);
	return 0;
	
}

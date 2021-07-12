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

void CBaseHandle::Init(int iEntry, int iSerialNumber)
{
	m_Index = iEntry | (iSerialNumber << NUM_ENT_ENTRY_BITS);
}

void CBaseHandle::Term( )
{
	m_Index = INVALID_EHANDLE_INDEX;
}

bool CBaseHandle::IsValid( ) const
{
	return m_Index != INVALID_EHANDLE_INDEX;
}

int CBaseHandle::GetEntryIndex( ) const
{
	return m_Index & ENT_ENTRY_MASK;
}

int CBaseHandle::GetSerialNumber( ) const
{
	return m_Index >> NUM_ENT_ENTRY_BITS;
}

int CBaseHandle::ToInt( ) const
{
	return (int)m_Index;
}

bool CBaseHandle::operator !=(const CBaseHandle& other) const
{
	return m_Index != other.m_Index;
}

bool CBaseHandle::operator ==(const CBaseHandle& other) const
{
	return m_Index == other.m_Index;
}

bool CBaseHandle::operator ==(const IHandleEntity* pEnt) const
{
	return Get( ) == pEnt;
}

bool CBaseHandle::operator !=(const IHandleEntity* pEnt) const
{
	return Get( ) != pEnt;
}

bool CBaseHandle::operator <(const CBaseHandle& other) const
{
	return m_Index<other.m_Index;
}

bool CBaseHandle::operator <(const IHandleEntity* pEntity) const
{
	unsigned long otherIndex = (pEntity) ? pEntity->GetRefEHandle( ).m_Index : INVALID_EHANDLE_INDEX;
	return m_Index < otherIndex;
}

const CBaseHandle& CBaseHandle::operator=(const IHandleEntity* pEntity)
{
	return Set(pEntity);
}

const CBaseHandle& CBaseHandle::Set(const IHandleEntity* pEntity)
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

IHandleEntity* CBaseHandle::Get( ) const
{
	(void)this;
	//return g_EntityList->GetClientEntityFromHandle(*this);
	BOOST_ASSERT(0);
	return 0;
	
}

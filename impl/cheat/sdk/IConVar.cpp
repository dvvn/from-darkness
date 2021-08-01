#include "IConVar.hpp"

using namespace cheat;
using namespace csgo;
using namespace hooks;

const char* ConVar::GetString( )
{
	return _Call_function(&ConVar::GetString, this, 11);
}

float ConVar::GetFloat( )
{
	return _Call_function(&ConVar::GetFloat, this, 12);
}

int ConVar::GetInt( )
{
	return _Call_function(&ConVar::GetInt, this, 13);
}

bool ConVar::GetBool( )
{
	return !!GetInt( );
}

void ConVar::SetValue(const char* value)
{
	_Call_function(&ConVar::SetValue_<decltype(value)>, this, 14, value);
}

void ConVar::SetValue(float value)
{
	_Call_function(&ConVar::SetValue_<decltype(value)>, this, 15, value);
}

void ConVar::SetValue(int value)
{
	_Call_function(&ConVar::SetValue_<decltype(value)>, this, 16, value);
}

ConVar::operator const char*( )
{
	return GetString( );
}

ConVar::operator float( )
{
	return GetFloat( );
}

ConVar::operator int( )
{
	return GetInt( );
}

ConVar::operator bool( )
{
	return GetBool( );
}

//float cheat::csgo::ConVar::GetValue( )
//{
//	DWORD xored = *(DWORD*)&m_pParent->m_Value.m_fValue ^ (DWORD)this;
//	return *(float*)&xored;
//}
//
//int cheat::csgo::ConVar::GetValueN( )
//{
//	return (int)(m_pParent->m_Value.m_nValue ^ (DWORD)this);
//}

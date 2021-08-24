#include "IConVar.hpp"

using namespace cheat;
using namespace csgo;

template <typename T>
static T _Get_helper(const ConVar* ptr, size_t index)
{
	return dhooks::_Call_function(&ConVar::get<T>, ptr, index);
}

template < >
const char* ConVar::get<const char*>( ) const
{
	return _Get_helper<const char*>(this, 11);
}

template < >
float ConVar::get<float>( ) const
{
	return _Get_helper<float>(this, 12);
}

template < >
int ConVar::get<int>( ) const
{
	return _Get_helper<int>(this, 13);
}

template < >
bool ConVar::get<bool>( ) const
{
	return !!this->get<int>( );
}

template <typename T>
static void _Set_helper(ConVar* ptr, size_t index, T value)
{
	return dhooks::_Call_function(static_cast<void(ConVar::*)(T)>(&ConVar::set), ptr, index, value);
}

void ConVar::set(const char* value)
{
	_Set_helper(this, 14, value);
	//dhooks::_Call_function(&ConVar::SetValue_<decltype(value)>, this, 14, value);
}

void ConVar::set(float value)
{
	_Set_helper(this, 15, value);
	//dhooks::_Call_function(&ConVar::SetValue_<decltype(value)>, this, 15, value);
}

void ConVar::set(int value)
{
	_Set_helper(this, 16, value);
	//dhooks::_Call_function(&ConVar::SetValue_<decltype(value)>, this, 16, value);
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

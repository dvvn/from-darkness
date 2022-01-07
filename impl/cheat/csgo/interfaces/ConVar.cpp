module;

module cheat.csgo.interfaces:ConVar;
import dhooks;

using namespace cheat::csgo;

template <typename T>
static void _Set_helper(ConVar* ptr, size_t index, T value)
{
	//return dhooks::_Call_function(static_cast<void(ConVar::*)(T)>(&ConVar::set), ptr, index, value);
	dhooks::call_function(&ConVar::set<T>, ptr, index, value);
}

template <typename T>
static T _Get_helper(const ConVar* ptr, size_t index)
{
	return dhooks::call_function(&ConVar::get<T>, ptr, index);
}

template < >
const char* ConVar::get( ) const { return _Get_helper<const char*>(this, 11); }

template < >
float ConVar::get( ) const { return _Get_helper<float>(this, 12); }

template < >
int ConVar::get( ) const { return _Get_helper<int>(this, 13); }

template < >
bool ConVar::get( ) const { return !!this->get<int>( ); }

template < >
void ConVar::set(const char* value) { _Set_helper(this, 14, value); }

template < >
void ConVar::set(float value) { _Set_helper(this, 15, value); }

template < >
void ConVar::set(int value) { _Set_helper(this, 16, value); }
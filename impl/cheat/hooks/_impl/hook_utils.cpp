#include "hook_utils.h"

using namespace cheat;
using namespace utl;
using namespace hooks;

//using func_type=method_info::func_type;

method_info::method_info(const type method_type, func_type&& func): type__(method_type),
																	storage__(move(func))
{
}

method_info::method_info(type method_type, LPVOID func_ptr): type__(method_type),
															 storage__(func_ptr)
{
	runtime_assert(func_ptr!=nullptr);
}

method_info::type method_info::get_type( ) const
{
	return type__;
}

LPVOID method_info::get( ) const
{
	runtime_assert(updated(), "Result isn't updated!");
	return std::get<LPVOID>(storage__);
}

bool method_info::update( )
{
	if (!updated( ))
	{
		auto& getter = (std::get<func_type>(storage__));
		// ReSharper disable once CppTooWideScope
		auto result = std::invoke(getter);

		if (result)
		{
			storage__ = result;
		}
		else
		{
			runtime_assert("Unable to update result!");
			return false;
		}
	}
	return true;
}

bool method_info::updated( ) const
{
	return visit(nstd::overload([](const func_type&)
								{
									return false;
								}, [](LPVOID ptr)
								{
									return true;
								}), storage__);
}

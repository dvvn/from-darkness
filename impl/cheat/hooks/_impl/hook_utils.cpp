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
	BOOST_ASSERT(func_ptr!=nullptr);
}

method_info::type method_info::get_type( ) const
{
	return type__;
}

LPVOID method_info::get( ) const
{
	BOOST_ASSERT_MSG(updated(), "Result isn't updated!");
	return ::get<LPVOID>(storage__);
}

bool method_info::update( )
{
	if (!updated( ))
	{
		auto& getter = (::get<func_type>(storage__));
		// ReSharper disable once CppTooWideScope
		auto result = invoke(getter);

		if (result)
		{
			storage__ = result;
		}
		else
		{
			BOOST_ASSERT("Unable to update result!");
			return false;
		}
	}
	return true;
}

bool method_info::updated( ) const
{
	return visit(overload([](const func_type&)
						  {
							  return false;
						  }, [](LPVOID ptr)
						  {
							  return true;
						  }), storage__);
}

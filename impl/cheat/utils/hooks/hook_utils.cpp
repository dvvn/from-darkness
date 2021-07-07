#include "hook_utils.h"

using namespace cheat::utl::hooks;

//using func_type=method_info::func_type;

method_info::method_info(const type method_type, const bool refresh_result, func_type&& func): type__(method_type),
																							   refresh_result__(refresh_result),
																							   updater__(move(func))
{
}

method_info::method_info(const type method_type, const bool refresh_result, const func_type& func): method_info(method_type, refresh_result, func_type(func))
{
}

method_info::type method_info::get_type( ) const
{
	return type__;
}

LPVOID method_info::get( ) const
{
	BOOST_ASSERT_MSG(result__ != nullptr, "Result isn't updated!");
	return result__;
}

bool method_info::update( )
{
	if (!result__ || refresh_result__)
	{
		result__ = updater__( );
		if (!result__)
		{
			BOOST_ASSERT("Unable to update result!");
			return false;
		}
	}
	return true;
}

bool method_info::updated( ) const
{
	return result__ != nullptr;
}

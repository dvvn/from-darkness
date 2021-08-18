#include "status.h"

using namespace cheat::hooks;

static constexpr auto _Status_to_string_cache = []
{

#define STATUS_STR(x) data[hook_status::x].first = /*CRYPT_STR*/(#x);

	auto data = std::array<std::pair<std::string_view, hook_status>, (hook_status::COUNT)>( );
	for (size_t i = 0; i < data.size( ); ++i)
	{
		data[i].second = i;
	}

	STATUS_STR(UNKNOWN)
	STATUS_STR(OK)
	//STATUS_STR(ERROR_ALREADY_INITIALIZED)
	//STATUS_STR(ERROR_NOT_INITIALIZED)
	STATUS_STR(ERROR_ALREADY_CREATED)
	STATUS_STR(ERROR_NOT_CREATED)
	STATUS_STR(ERROR_ENABLED)
	STATUS_STR(ERROR_DISABLED)
	STATUS_STR(ERROR_NOT_EXECUTABLE)
	STATUS_STR(ERROR_UNSUPPORTED_FUNCTION)
	STATUS_STR(ERROR_MEMORY_ALLOC)
	STATUS_STR(ERROR_MEMORY_PROTECT)
	STATUS_STR(ERROR_MODULE_NOT_FOUND)
	STATUS_STR(ERROR_FUNCTION_NOT_FOUND)
	STATUS_STR(ERROR_VTABLE_NOT_FOUND)
	STATUS_STR(ERROR_NOT_READABLE)

#undef STATUS_STR

#ifdef _DEBUG
	for (const auto& key: data | ranges::views::keys)
	{
		if (key == std::remove_cvref_t<decltype(key)>( ))
		{
			if (std::is_constant_evaluated( ))
				throw std::exception("Element isn't set!");
			else
				runtime_assert("Element isn't set!");
		}
	}
#endif

	return data;
}( );

std::string_view hook_status::to_string(hook_status status) const
{
	try
	{
		return _Status_to_string_cache[(status.value_raw())].first;
	}
	catch (...)
	{
		return /*CRYPT_STR*/"Unsupported status";
	}
}

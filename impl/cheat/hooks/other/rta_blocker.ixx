module;

#include <dhooks/includes.h>
#include <string_view>

export module cheat.hooks.other:rta_blocker;
export import dhooks;

namespace cheat::hooks::other
{
	//return address blocker

	void* rta_blocker_get_target_method(const std::wstring_view& module_name);

	export template<size_t Index>
		class rta_blocker final :public dhooks::select_hook_holder<char(std::false_type::*)(const char*), Index>
	{
	public:
		bool load(const std::wstring_view& module_name)
		{
			this->set_target_method(rta_blocker_get_target_method(module_name));
			return this->hook( ) && this->enable( );
		}

	protected:
		void callback([[maybe_unused]] const char* module_name) override
		{
			this->store_return_value(1);
		}
	};
}

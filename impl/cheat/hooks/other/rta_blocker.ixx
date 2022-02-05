module;

#include "cheat/hooks/base_includes.h"
#include <string>

export module cheat.hooks.other:rta_blocker;
export import cheat.hooks.base;

namespace cheat::hooks::other
{
	//return address blocker

	void* rta_blocker_get_target_method(const std::wstring_view& module_name);

	export template<size_t Index>
		class rta_blocker final :public hook_base<rta_blocker<Index>, char(std::false_type::*)(const char*), Index>
	{
	public:
		rta_blocker(const std::wstring_view& module_name)
			:module_name_(module_name)
		{
		}

	protected:

		void construct( ) noexcept override
		{
			this->set_target_method(rta_blocker_get_target_method(module_name_));
		}

		void callback([[maybe_unused]] const char* module_name) override
		{
			this->store_return_value(1);
		}

	private:
		std::wstring module_name_;
	};
}

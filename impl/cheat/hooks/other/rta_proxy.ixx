module;

#include "cheat/hooks/base_includes.h"
#include <string>

export module cheat.hooks.other:rta_proxy;
export import cheat.hooks.base;

namespace cheat::hooks::other
{
	//return address blocker
	//all proxy funcs use same callback
	export struct rta_proxy : hook_base<rta_proxy, char(std::false_type::*)(const char*)>
	{
		rta_proxy(const std::wstring_view& module_name);
				
	protected:
		void construct( ) noexcept final;
		void* get_target_method( ) const override;
		void callback(const char* module_name) override;
	private:
		std::wstring module_name_;
	};
}

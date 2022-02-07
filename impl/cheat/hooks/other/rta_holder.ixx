module;

#include "cheat/service/basic_includes.h"
#include <vector>

export module cheat.hooks.other:rta_holder;
export import cheat.service;
export import dhooks;

namespace cheat::hooks::other
{
	export class rta_holder final :
		public dynamic_service<rta_holder>
		, public dhooks::hook_disabler_lazy //for unload
	{
		using storage_type = std::vector<std::unique_ptr<dhooks::hook_holder_data>>;
		storage_type proxies_;
	protected:
		void construct( ) noexcept override;
		bool load( ) noexcept override;

	public:
		void request_disable( ) override;
	};
}

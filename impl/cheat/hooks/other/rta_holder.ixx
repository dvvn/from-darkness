module;

#include "cheat/service/basic_includes.h"
#include <vector>

export module cheat.hooks.other:rta_holder;
export import cheat.service;
import dhooks;

namespace cheat::hooks::other
{
	export class rta_holder final : public dynamic_service<rta_holder>
	{
		using storage_type = std::vector<std::unique_ptr<dhooks::hook_holder_data>>;
		storage_type proxies_;
	protected:
		void construct( ) noexcept override;
		bool load( ) noexcept override;
	};
}

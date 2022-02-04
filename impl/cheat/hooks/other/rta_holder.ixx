module;

#include "cheat/hooks/base_includes.h"
#include <vector>

export module cheat.hooks.other:rta_holder;
import :rta_proxy;

namespace cheat::hooks::other
{	
	export class rta_holder final : public dynamic_service<rta_holder>
	{
		using storage_type=std::vector<rta_proxy>;

		storage_type proxies_;
	protected:
		void construct( ) noexcept override;
		bool load( ) noexcept override;
	};
}

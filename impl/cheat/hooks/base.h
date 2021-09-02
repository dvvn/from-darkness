#pragma once
#include "cheat/core/service.h"

#include "detour hook/hook_utils.h"

namespace cheat::hooks
{
	namespace detail
	{
		class helper : public virtual service_base, public virtual dhooks::hook_holder_base
		{
		protected:
			load_result      load_impl( ) override;
			std::string_view object_name( ) const final;
			void             reset( ) final;
		};
	}

	template <typename Proxy, typename Target>
	struct hook_base : service<Proxy>
		, dhooks::_Detect_hook_holder_t<Target>
		, detail::helper
	{
	protected:
		std::string_view name( ) const final { return nstd::type_name<Proxy, "cheat::hooks">; }
	};
}

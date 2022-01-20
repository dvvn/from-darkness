module;

#include "cheat/service/basic_includes.h"
#include "storage_includes.h"

export module cheat.netvars;
export import cheat.service;
import :storage;
#ifdef CHEAT_NETVARS_RESOLVE_TYPE
import :lazy;
#endif

namespace cheat
{
	using namespace netvars_impl;

	export class netvars final : public dynamic_service<netvars>
	{
	public:
		~netvars( ) override;
		netvars( );

		int at(const std::string_view& table, const std::string_view& item) const;

	protected:
		void construct( ) noexcept override;
		bool load( ) noexcept override;

	private:
		netvars_storage storage_;
#ifdef CHEAT_NETVARS_RESOLVE_TYPE
		lazy::files_storage lazy_;
#endif
	};
}

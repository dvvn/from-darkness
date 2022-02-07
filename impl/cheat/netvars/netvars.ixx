module;

#include "cheat/service/basic_includes.h"

export module cheat.netvars;
export import cheat.service;

namespace cheat
{
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
		struct impl;
		std::unique_ptr<impl>impl_;
	};
}

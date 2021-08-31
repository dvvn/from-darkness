#pragma once

#include "cheat/core/service.h"

namespace cheat
{
	class netvars final: public service<netvars>
					   , service_sometimes_skipped
	{
	public:
		~netvars( ) override;
		netvars( );

		int at(const std::string_view& table, const std::string_view& item) const;

		struct hidden;

	protected:
		load_result load_impl( ) override;
		void        after_load( ) override;

	private:
		std::unique_ptr<hidden> hidden_;
	};
}

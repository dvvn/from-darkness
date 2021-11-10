#pragma once

#include "cheat/service/include.h"

namespace cheat
{
	class netvars_impl final : public service<netvars_impl>
	{
	public:
		~netvars_impl( ) override;
		netvars_impl( );

		int at(const std::string_view& table, const std::string_view& item) const;

	protected:
		load_result load_impl( ) noexcept override;

	private:
		struct data_type;
		std::unique_ptr<data_type> data_;
	};

	CHEAT_SERVICE_SHARE(netvars);
}

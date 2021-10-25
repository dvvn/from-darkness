#pragma once

#include "cheat/core/service.h"

namespace cheat
{
	namespace detail::netvars
	{
		struct netvars_data;
	}

	class netvars final : public service_instance_shared<netvars>
	{
	public:
		~netvars( ) override;
		netvars( );

		int at(const std::string_view& table, const std::string_view& item) const;

	protected:
		load_result load_impl( ) noexcept override;

	private:
		using data_type=detail::netvars::netvars_data;
		std::unique_ptr<data_type> data_;
	};
}

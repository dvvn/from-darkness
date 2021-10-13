#pragma once

#include "cheat/core/service.h"

namespace cheat
{
	namespace detail
	{
		struct netvars_data;
	}

	class netvars final : public service<netvars>
	{
	public:
		~netvars() override;
		netvars();

		int at(const std::string_view& table, const std::string_view& item) const;

	protected:
		load_result load_impl() noexcept override;

	private:
		std::unique_ptr<detail::netvars_data> data_;
	};
}

#pragma once

#include "cheat/core/service.h"

namespace cheat
{
	class netvars final : public service<netvars>
	{
	public:
		~netvars() override;
		netvars();

		int at(const std::string_view& table, const std::string_view& item) const;

		struct hidden;

	protected:
		load_result load_impl() noexcept override;

	private:
		std::unique_ptr<hidden> hidden_;
	};
}

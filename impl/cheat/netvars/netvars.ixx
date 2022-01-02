module;

#include "cheat/service/includes.h"

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
		bool load_impl( ) noexcept override;

	private:
		struct data_type;
		std::unique_ptr<data_type> data_;
	};

	//CHEAT_SERVICE_SHARE(netvars);
}

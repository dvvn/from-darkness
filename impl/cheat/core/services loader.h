#pragma once
#include "service.h"

namespace std
{
	class stop_token;
}

// ReSharper disable CppInconsistentNaming
struct HINSTANCE__;
using HMODULE = HINSTANCE__*;
// ReSharper restore CppInconsistentNaming

namespace cheat
{
	class services_loader final : public service<services_loader>
	{
#ifndef CHEAT_GUI_TEST
		using service_base::load;
#endif
	public:
		~services_loader() override;
		services_loader();

#ifndef CHEAT_GUI_TEST
		HMODULE my_handle( ) const;
		void    load(HMODULE handle);
		void    unload( );
		std::stop_token load_thread_stop_token() const;
#endif
		static executor make_executor();

	protected:
		load_result load_impl() noexcept override;

	private:
#ifndef CHEAT_GUI_TEST
		HMODULE my_handle__ = nullptr;
		struct load_thread;
		std::unique_ptr<load_thread> load_thread_;
#endif

#if 0
		template <initializable_service T>
		T* find_service( ) const
		{

			for(const stored_service& service : storage_ | ranges::views::elements<0>)
			{
				if(service->type( ) == typeid(T))
					return service.get( );
			}

			return nullptr;
		}

		template <initializable_service T>
		services_loader& load_service(bool lock = false)
		{
			ranges::find_if(storage_, [](const load_info& info) { return info.service })
		}

		std::vector<load_info> storage_;
#endif
	};

#define CHEAT_REGISTER_SERVICE(_TYPE_)\
	[[maybe_unused]]\
	static const auto _CONCAT(_Unused,__LINE__) = (cheat::services_loader::get_ptr()->wait_for_service<_TYPE_>(true), 0)
}

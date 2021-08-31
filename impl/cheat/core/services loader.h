#pragma once
#include "service.h"

namespace std
{
	class stop_token;
}

namespace cheat
{
	class services_loader final: public service_core<services_loader>, public nstd::one_instance<services_loader>
	{
#ifndef CHEAT_GUI_TEST
		using service_base::load;
#endif
	public:
		~services_loader( ) override;
		services_loader( );

#ifndef CHEAT_GUI_TEST
		HMODULE my_handle( ) const;
		void    load(HMODULE handle);
		void    unload( );
#endif

		std::stop_token load_thread_stop_token( ) const;

	protected:
		load_result load_impl( ) override;
		void        after_load( ) override;

	private:
#ifndef CHEAT_GUI_TEST
		HMODULE      my_handle__ = nullptr;
		std::jthread load_thread_;
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
}

#pragma once
#include "cheat/service/include.h"

#include <thread>

namespace dhooks
{
	class hook_holder_base;
}

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
	class services_loader final : public service<services_loader>, public nstd::one_instance<services_loader>
	{
#ifndef CHEAT_GUI_TEST
		using basic_service::load;
#endif
	public:
		~services_loader( ) override;
		services_loader( );

		services_loader(services_loader&& other) =default;
		services_loader& operator=(services_loader&& other) =default;

#ifndef CHEAT_GUI_TEST
		HMODULE my_handle( ) const;
		void load(HMODULE handle);
		void unload_delayed( );
		std::stop_token load_thread_stop_token( ) const;
#endif

		std::shared_ptr<executor> get_executor(size_t threads_count = std::thread::hardware_concurrency( ));

		std::vector<stored_service<dhooks::hook_holder_base>> get_hooks(bool steal);

	protected:
		load_result load_impl( ) noexcept override;

	private:
		std::weak_ptr<executor> executor_;

#ifndef CHEAT_GUI_TEST
		HMODULE my_handle__ = nullptr;
		std::jthread load_thread_;
#endif

#if 0
		template <initializable_service T>
		T* find_service( ) const
		{

			for(const stored_service& service_instance_shared : storage_ | ranges::views::elements<0>)
			{
				if(service_instance_shared->type( ) == typeid(T))
					return service_instance_shared.get( );
			}

			return nullptr;
		}

		template <initializable_service T>
		services_loader& load_service(bool lock = false)
		{
			ranges::find_if(storage_, [](const load_info& info) { return info.service_instance_shared })
		}

		std::vector<load_info> storage_;
#endif
	};

#define CHEAT_SERVICE_REGISTER(_NAME_)\
	__pragma(message("Service \""#_NAME_"\" registered at " __TIME__))\
	[[maybe_unused]]\
	static const auto _CONCAT(_Unused,__LINE__) = (_NAME_::get(), static_cast<std::byte>(0))
}

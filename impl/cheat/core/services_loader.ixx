module;

#include <dhooks/wrapper.h>

#include <Windows.h>

#include <vector>

export module cheat.core.services_loader;
import cheat.core.service;

export namespace cheat
{
	using all_hooks_storage = std::vector<stored_service<dhooks::hook_holder_base>>;

	class services_loader final : public static_service<services_loader>
	{
#ifndef CHEAT_GUI_TEST
		using basic_service::load;
#endif
	public:
		~services_loader( ) override;
		services_loader( );

		services_loader(services_loader&& other) = default;
		services_loader& operator=(services_loader&& other) = default;

		bool root_class( )const override { return true; }

#ifndef CHEAT_GUI_TEST
		HMODULE my_handle( ) const;
		void load(HMODULE handle);
		void unload_delayed( );
		std::stop_token load_thread_stop_token( ) const;
#else
		bool load( );
#endif
		using executor_shared = std::shared_ptr<executor>;

		executor_shared get_executor(size_t threads_count);
		executor_shared get_executor( );

		all_hooks_storage get_hooks(bool steal);

	protected:
		load_result load_impl( ) noexcept override;

	private:
		std::weak_ptr<executor> executor_;
#ifndef CHEAT_GUI_TEST
		HMODULE own_handle_ = nullptr;
		std::jthread load_thread_;
#endif
	};

//#define CHEAT_SERVICE_REGISTER(_NAME_)\
//	__pragma(message("Service \""#_NAME_"\" registered at " __TIME__))\
//	[[maybe_unused]]\
//	static const auto _CONCAT(_Unused,__LINE__) = (_NAME_::get(), static_cast<std::byte>(0))
}

#pragma once
#include "cheat/service/include.h"

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
	class all_hooks_storage
	{
	public:
		using value_type = stored_service<dhooks::hook_holder_base>;

		all_hooks_storage( );
		~all_hooks_storage( );

		all_hooks_storage(all_hooks_storage&&) noexcept;
		all_hooks_storage& operator=(all_hooks_storage&&) noexcept;

		value_type* begin( ) const;
		value_type* end( ) const;

		void push_back(value_type&& val);
		void push_back(const value_type& val);

		bool empty( ) const;
		size_t size( ) const;

		void clear();

	private:
		struct impl;
		std::unique_ptr<impl> impl_;
	};

	class services_loader final : public service<services_loader, true>, public nstd::one_instance<services_loader>
	{
#ifndef CHEAT_GUI_TEST
		using basic_service::load;
#endif
	public:
		~services_loader( ) override;
		services_loader( );

		services_loader(services_loader&& other)            = default;
		services_loader& operator=(services_loader&& other) = default;

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

		struct impl;
		std::unique_ptr<impl>impl_;




	};

#define CHEAT_SERVICE_REGISTER(_NAME_)\
	__pragma(message("Service \""#_NAME_"\" registered at " __TIME__))\
	[[maybe_unused]]\
	static const auto _CONCAT(_Unused,__LINE__) = (_NAME_::get(), static_cast<std::byte>(0))
}

#pragma once
#include "service.h"

namespace cheat
{
	namespace detail
	{
		template <typename T>
		concept awaitable_service = std::derived_from<T, service_base> &&
									requires( ) { { T::get_ptr_shared( ) }->std::convertible_to<std::shared_ptr<service_base>>; };

		class services_holder final
		{
		public:
			std::future<bool> load( );

			template <class ...T>
				requires(awaitable_service<T> && ...)
			services_holder& load(bool dont_wait = false)
			{
				Add_to_storage_<T...>(dont_wait ? services_dont_wait__ : services__, true);
				return *this;
			}

			template <class ...T>
				requires(awaitable_service<T> && ...)
			services_holder& wait( )
			{
				Add_to_storage_<T...>(services_wait_for__, false);
				return *this;
			}

			services_holder& then( );

			using all_hooks_storage = nstd::unordered_set<dhooks::hook_holder_base*>;

			all_hooks_storage get_all_hooks( );

		private:
			using services_storage = std::vector<std::shared_ptr<service_base>>;

			template <class ...T>
			static void Add_to_storage_(services_storage& storage, bool steal)
			{
				constexpr auto add_count = sizeof...(T);
				if constexpr (add_count > 1)
					storage.reserve(storage.size( ) + add_count);

				(storage.emplace_back(T::get_ptr_shared(steal)), ...);
			}

			services_storage services__,
							 services_dont_wait__,
							 services_wait_for__;

			std::unique_ptr<services_holder> next__;
		};
	}

	class services_loader final: public service_base, public nstd::one_instance<services_loader>
	{
#ifndef CHEAT_GUI_TEST
		using service_base::load;
#endif
	public:
		~services_loader( ) override;
		services_loader( );

		std::string_view name( ) const override;

#ifndef CHEAT_GUI_TEST
		HMODULE my_handle( ) const;
		void    load(HMODULE handle);
		void    unload( );

		bool unloaded( ) const;
#endif
		void reset( );

	protected:
		bool load_impl( ) override;
		void after_load( ) override;

	private:
#ifndef CHEAT_GUI_TEST
		HMODULE           my_handle__ = nullptr;
		std::atomic<bool> unloaded_   = false;
#endif
		detail::services_holder services__;
	};
}

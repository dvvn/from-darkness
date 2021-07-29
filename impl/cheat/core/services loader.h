#pragma once
#include "service.h"

namespace cheat
{
	namespace detail
	{
		template <typename T>
		concept awaitable_service2 = std::derived_from<T, service_base2> &&
									 requires( ) { { T::get_ptr( ) }->std::convertible_to<utl::shared_ptr<service_base2>>; };

		class services_holder final
		{
		public:
			utl::future<bool> load( );

			template <class ...T>
				requires(awaitable_service2<T> && ...)
			services_holder& load(bool dont_wait = false)
			{
				Add_to_storage_<T...>(dont_wait ? services_dont_wait__ : services__);
				return *this;
			}

			template <class ...T>
				requires(awaitable_service2<T> && ...)
			services_holder& wait( )
			{
				Add_to_storage_<T...>(services_wait_for__);
				return *this;
			}

			services_holder& then( )
			{
				if (!next__)
					next__ = utl::make_unique<services_holder>( );

				return *next__;
			}

			utl::unordered_set<hooks::hook_holder_base*> get_all_hooks( );

		private:
			template <class ...T>
			static void Add_to_storage_(utl::vector<utl::shared_ptr<service_base2>>& where)
			{
				if constexpr (constexpr auto add_count = sizeof...(T); add_count > 1)
					where.reserve(where.size( ) + add_count);

				(where.emplace_back(T::get_ptr( )), ...);
			}

			using services_storage_type = utl::vector<utl::shared_ptr<service_base2>>;

			services_storage_type services__, services_dont_wait__, services_wait_for__;
			utl::unique_ptr<services_holder> next__;
			bool locked__ = false;
		};
	}

	class services_loader final: public service_base2, public utl::one_instance<services_loader>
	{
#ifndef CHEAT_GUI_TEST
		using service_base2::load;
#endif
	public:
		services_loader( );

		utl::string_view name( ) const override;

#ifndef CHEAT_GUI_TEST
		HMODULE my_handle( ) const;
		void load(HMODULE handle);
		void unload( );
#endif
		void reset( );

	protected:
		bool Do_load( ) override;

	private:
#ifndef CHEAT_GUI_TEST
		HMODULE my_handle__ = nullptr;
#endif
		detail::services_holder services__;
	};
}

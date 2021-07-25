#pragma once
#include "helpers.h"

namespace cheat
{
	class root_service;

	namespace detail
	{
		class service_base;
		class async_service;
		class sync_service;

		template <typename T>
		concept awaitable_service = std::derived_from<T, service_base> &&
									requires( ) { { T::get_shared( ) }->std::convertible_to<utl::shared_ptr<service_base>>; };

		class service_base: utl::noncopyable
		{
		public:
			friend class async_service;
			friend class sync_service;
			friend class root_service;

			using loader_type = utl::thread_pool;
			//using loader_type_shared = boost::shared_ptr<loader_type>;
			using service_shared = utl::shared_ptr<service_base>;
			using load_task_type = utl::shared_future<void>;
			using wait_for_storage_type = utl::unordered_set<service_shared>;

			virtual ~service_base( );
			service_base( );

			load_task_type init(loader_type& loader);
			void reset( );

			bool initialized( ) const;

			virtual utl::string_view debug_name( ) const = 0;

		protected:
			virtual load_task_type Initialize(loader_type& loader) = 0;
			virtual void Load( ) = 0;
			virtual utl::string Get_loaded_message( ) const;
			utl::string Get_loaded_message_disabled( ) const;
			virtual void Post_load( );

			template <awaitable_service S>
			void Wait_for( )
			{
				Wait_for_add_impl_(S::get_shared( ));
			}

			template <awaitable_service S>
			void Wait_for_weak( )
			{
				auto s = S::get_shared( );
				if (Find_recursuve_(s))
					return;
				Wait_for_add_impl_(utl::move(s));
			}

		private:
			void Print_loaded_message_( ) const;
			void Wait_for_add_impl_(service_shared&& service);
			bool Find_recursuve_(const service_shared& service) const;

			load_task_type load_task__;
			utl::thread::id creator__;
			wait_for_storage_type wait_for__;

			void Waiting_task_assert_( ) const;
		};

		class async_service: public service_base
		{
		protected:
			load_task_type Initialize(loader_type& loader) final;
		};

		class sync_service: public service_base
		{
		protected:
			load_task_type Initialize(loader_type& loader) final;
		};
	}

	enum class service_mode
	{
		sync = 0,
		async
	};

	namespace detail
	{
		template <service_mode Mode>
		using service_type_selector = std::conditional_t<Mode == service_mode::sync, sync_service, async_service>;
	}

	template <typename T, service_mode Mode>
	class service_shared: public detail::service_type_selector<Mode>, public utl::one_instance_shared<T, static_cast<size_t>(Mode)>
	{
	public:
		utl::string_view debug_name( ) const final
		{
			return _Type_name<T>(false);
		}
	};

	template <typename T, service_mode Mode>
	class service_static: public detail::service_type_selector<Mode>, public utl::one_instance<T, static_cast<size_t>(Mode)>
	{
	public:
		utl::string_view debug_name( ) const final
		{
			return _Type_name<T>(false);
		}
	};
}

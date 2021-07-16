#pragma once

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

		template <typename T>
		struct type_name
		{
		private:
			static constexpr utl::string_view Get_( ) noexcept
			{
				const auto full_name = utl::string_view(__FUNCSIG__);
				const auto left_marker = utl::string_view("type_name<");
				const auto right_marker = utl::string_view(">::Get_");

				const auto left_marker_index = full_name.find(left_marker);
				//static_assert(left_marker_index != std::string_view::npos);
				const auto start_index = left_marker_index + left_marker.size( );
				const auto end_index = full_name.find(right_marker, left_marker_index);
				//static_assert(end_index != std::string_view::npos);
				const auto length = end_index - start_index;

				//class cheat::A::B:: name
				const auto obj_name = full_name.substr(start_index, length);
				const auto left_marker2 = utl::string_view("cheat::");
				if (const auto left_marker_index2 = obj_name.find(left_marker2); left_marker_index2 != utl::string_view::npos)
					return obj_name.substr(left_marker_index2 + left_marker2.size( ));

				const auto space_index = obj_name.find(' ');
				return space_index == utl::string_view::npos ? obj_name : obj_name.substr(space_index + 1);
			}

		public:
			static constexpr utl::string_view name = Get_( );
		};
	}

	template <typename T, service_mode Mode>
	class service_shared: public detail::service_type_selector<Mode>, public utl::one_instance_shared<T, static_cast<size_t>(Mode)>
	{
	public:
		utl::string_view debug_name( ) const final
		{
#ifdef CHEAT_DEBUG_MODE
			return detail::type_name<T>::name;
#else
			throw;
#endif
		}
	};

	template <typename T, service_mode Mode>
	class service_static: public detail::service_type_selector<Mode>, public utl::one_instance<T, static_cast<size_t>(Mode)>
	{
	public:
		utl::string_view debug_name( ) const final
		{
#ifdef CHEAT_DEBUG_MODE
			return detail::type_name<T>::name;
#else
			throw;
#endif
		}
	};
}

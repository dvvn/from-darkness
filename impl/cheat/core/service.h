#pragma once

namespace cheat
{
	//use for frequently called services
	struct service_top_level_only_tag
	{
	};

	namespace detail
	{
		class service_base;

		template <typename T>
		concept awaitable_service_strong = std::derived_from<T, service_base> &&
										   !std::derived_from<T, service_top_level_only_tag> &&
										   requires( ) { { T::get_shared( ) }->std::convertible_to<utl::shared_ptr<service_base>>; };
		template <typename T>
		concept awaitable_service = std::derived_from<T, service_base> &&
									requires( ) { { T::get_shared( ) }->std::convertible_to<utl::shared_ptr<service_base>>; };

		class service_base: utl::noncopyable
		{
		public:
			friend class async_service;
			friend class sync_service;

			using loader_type = utl::thread_pool;
			//using loader_type_shared = boost::shared_ptr<loader_type>;
			using service_shared = utl::shared_ptr<service_base>;
			using load_task_type = utl::shared_future<void>;
			using wait_for_storage_type = utl::unordered_set<service_shared>;

			virtual ~service_base( );
			service_base( );

			load_task_type init(loader_type& loader);
			void           reset( );

			bool initialized( ) const;

			virtual utl::string_view debug_name( ) const =0;

		protected:
			virtual load_task_type Initialize(loader_type& loader) =0;
			virtual void           Load( ) =0;
			virtual utl::string    Get_loaded_message( ) const;
			virtual void           Post_load( );

			wait_for_storage_type& Storage( );

			void Wait_for_add_impl(service_shared&& service);
		private:
			void Print_loaded_message_( ) const;
			bool Find_recursuve_(const service_shared& service) const;

			load_task_type        load_task__;
			utl::thread::id       creator__;
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
		sync=0,
		async
	};

	namespace detail
	{
		template <service_mode Mode>
		using service_type_selector = std::conditional_t<Mode == service_mode::sync, sync_service, async_service>;

		template <typename T>
		struct type_name
		{
			static constexpr utl::string_view get( ) noexcept
			{
				const auto full_name = utl::string_view(__FUNCSIG__);
				const auto left_marker = utl::string_view("type_name<");
				const auto right_marker = utl::string_view(">::get");

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

			static constexpr utl::string_view name = get( );
		};
	}

	template <typename T, service_mode Mode>
	class service_shared: public detail::service_type_selector<Mode>, public utl::one_instance_shared<T, static_cast<size_t>(Mode)>
	{
	public:
		static_assert(!std::derived_from<T, service_top_level_only_tag>);

		utl::string_view debug_name( ) const final
		{
#ifdef CHEAT_DEBUG_MODE
			return detail::type_name<T>::get( );
#else
			throw;
#endif
		}

	protected:
		template <detail::awaitable_service_strong S>
		void Wait_for( )
		{
			this->Wait_for_add_impl(S::get_shared( ));
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

	protected:
		template <detail::awaitable_service S>
		void Wait_for( )
		{
			this->Wait_for_add_impl(S::get_shared( ));
		}
	};
}

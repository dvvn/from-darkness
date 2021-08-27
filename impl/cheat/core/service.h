#pragma once

namespace cheat
{
	struct service_state final
	{
		enum value_type :uint8_t
		{
			unset = 0,
			moved,
			waiting,
			loading,
			loaded,
			skipped,
			error,
		};

		NSTD_ENUM_STRUCT(service_state, unset)
	};

	class service_base;
	template <typename T>
	concept root_service = std::derived_from<T, service_base>;

	template <typename T>
	concept derived_service = root_service<T> && std::default_initializable<T>;

	template <typename T>
	concept shared_service = derived_service<T> && requires
	{
		T::get_ptr( );
		T::get_ptr_weak( );
		T::get_ptr_shared( );
	};

	class service_base
	{
	public:
		virtual ~service_base( );

		virtual std::string_view      name( ) const = 0;
		virtual const std::type_info& type( ) const = 0;

		service_base( ) = default;

		service_base(const service_base&)            = delete;
		service_base& operator=(const service_base&) = delete;

		service_base(service_base&& other) noexcept;
		void operator=(service_base&& other) noexcept;

		using load_result = cppcoro::task<service_state>;
		using child_wait_result = cppcoro::task<bool>;

		using executor = cppcoro::static_thread_pool;

		using shared_service = std::shared_ptr<service_base>;

		shared_service find_service(const std::type_info& info) const
		{
			for (const auto& service: services_)
			{
				if (service->type( ) == info)
					return service;
			}

			return { };
		}

		template <derived_service T>
		auto find_service( ) const
		{
			const auto found = find_service(typeid(T));
			return std::dynamic_pointer_cast<T>(found);
		}

		void store_service(shared_service&& srv, const std::type_info& info)
		{
			runtime_assert(find_service(info) == shared_service(), "Service already stored!");
			services_.push_back(std::move(srv));
		}

		template <derived_service T>
		void add_service( )
		{
			store_service(std::make_shared<T>( ), typeid(T));
		}

		template <cheat::shared_service T>
		void add_service( )
		{
			store_service(T::get_ptr_shared(true), typeid(T));
		}

		service_state state( ) const;

		void reset( );

	protected:
		bool              validate_init_state( ) const;
		child_wait_result wait_for_others(executor& ex);
	public:
		virtual load_result load(executor& ex);
	protected:
		virtual load_result load_impl( ) = 0;

		virtual void after_load( );
		virtual void after_skip( );
		virtual void after_error( );

		void set_state(service_state&& state);

	public:
		std::span<const shared_service> services( ) const;

	private:
		service_state               init_state_, state_;
		std::vector<shared_service> services_;
	};

	template <typename T>
	class service_core: public virtual service_base
	{
	public:
		std::string_view name( ) const final
		{
			return nstd::type_name<T, "cheat">( );
		}

		const std::type_info& type( ) const final
		{
			return typeid(T);
		}
	};

	template <typename T>
	class service: public service_core<T>, public nstd::one_instance_shared<T>
	{
	};

	class service_hook_helper: public virtual service_base, public virtual dhooks::hook_holder_base
	{
	protected:
		load_result load_impl( ) override;
	};

	class service_always_skipped: public virtual service_base
	{
	protected:
		load_result load(executor& ex) final;
	};
}

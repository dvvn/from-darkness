#pragma once

//#include "detour hook/hook_utils.h"

#include "nstd/enum as struct.h"
#include "nstd/one_instance.h"
#include "nstd/type name.h"

#include <cppcoro/task.hpp>

#include <concepts>
#include <span>

namespace cppcoro
{
	class static_thread_pool;
	/*template <typename T>
	class [[nodiscard]] task;*/
}

namespace cheat
{
	struct service_state final
	{
		enum value_type : std::uint8_t
		{
			unset = 0,
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

	struct service_base_fields
	{
		service_base_fields( );
		~service_base_fields( );

		service_base_fields(service_base_fields&& other) noexcept;
		service_base_fields& operator=(service_base_fields&& other) noexcept;

		service_state state;

		struct storage_type;
		std::unique_ptr<storage_type> deps; //dependencies what must be loaded before
	};

	class service_base
	{
	public:
		virtual ~service_base( ) = default;
		service_base( )          = default;

		virtual std::string_view      name( ) const = 0;
		virtual const std::type_info& type( ) const = 0;

		virtual std::string_view object_name( ) const;

		using load_result = cppcoro::task<service_state>;
		using child_wait_result = cppcoro::task<bool>;

		using executor = cppcoro::static_thread_pool;

		using stored_service = std::shared_ptr<service_base>;

		std::span<const stored_service> services( ) const;

		stored_service find_service(const std::type_info& info) const;

		template <derived_service T>
		auto find_service( ) const
		{
			const auto found = find_service(typeid(T));
			return std::dynamic_pointer_cast<T>(found);
		}

		void store_service(stored_service&& srv, const std::type_info& info);

		template <derived_service T>
		void add_service( )
		{
			store_service(std::make_shared<T>( ), typeid(T));
		}

		template <shared_service T>
		void add_service( )
		{
			store_service(T::get_ptr_shared(true), typeid(T));
		}

		service_state state( ) const;

		virtual void reset( );

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

		virtual const service_base_fields& fields( ) const =0;
		virtual service_base_fields&       fields( ) =0;
	};

	class service_sometimes_skipped: public virtual service_base
	{
	public:
		service_sometimes_skipped(bool skip);
		load_result load(executor& ex) final;

		bool always_skipped( ) const;

	private:
		bool skipped_;
	};

	template <typename T>
	class service_core: public virtual service_base
	{
	public:
		std::string_view      name( ) const final { return nstd::type_name<T, "cheat">( ); }
		const std::type_info& type( ) const final { return typeid(T); }

	protected:
		const service_base_fields& fields( ) const final { return fileds_; }
		service_base_fields&       fields( ) final { return fileds_; }

	private:
		service_base_fields fileds_;
	};

	template <typename T>
	class service: public service_core<T>, public nstd::one_instance_shared<T>
	{
	};
}

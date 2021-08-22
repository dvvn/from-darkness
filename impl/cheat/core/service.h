#pragma once
#include "helpers.h"

namespace cheat
{
	struct service_state final
	{
		enum value_type :uint8_t
		{
			unset = 0,
			moved,
			skipped,
			loading,
			loaded,
#ifdef BOOST_THREAD_PROVIDES_INTERRUPTIONS
						stopped,
#endif
			error
		};

		bool operator!( ) const;
		bool done( ) const;
		bool disabled( ) const;

		NSTD_ENUM_STRUCT(service_state, unset)
	};

	class service_base
	{
	public:
		friend class service_always_skipped;

		virtual                  ~service_base( );
		virtual std::string_view name( ) const = 0;

		service_base( ) = default;

		service_base(const service_base&)            = delete;
		service_base& operator=(const service_base&) = delete;

		service_base(service_base&& other) noexcept;
		void operator=(service_base&& other) noexcept;

		service_state state( ) const;
		virtual void  load( );

	protected:
		/**
		 * \brief
		 * \return true: loaded, false: skipped.
		 * throw exception if something wrong!
		 */
		virtual bool load_impl( ) = 0;

		virtual void after_load( )
		{
		}
#if defined BOOST_THREAD_PROVIDES_INTERRUPTIONS
		virtual void after_stop( )
		{			
		}
#endif
		virtual void after_error(const std::exception&)
		{
		}

	private:
		std::atomic<service_state> state_;
	};

	template <typename T>
	class service: public virtual service_base, public utl::one_instance_shared<T>
	{
	public:
		std::string_view name( ) const final
		{
			return type_name<T>(false);
		}
	};

	class service_hook_helper: public virtual service_base, public virtual hooks::hook_holder_base
	{
	protected:
		bool load_impl( ) override;
	};

	class service_always_skipped: public virtual service_base
	{
	public:
		service_always_skipped( );
		void load( ) final;
	};
}

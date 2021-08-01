#pragma once
#include "helpers.h"

namespace cheat
{
	struct service_state2 final
	{
		enum value_type :uint8_t
		{
			unset = 0,
			moved,
			loading,
			loaded,
			stopped,
			error
		};

		bool operator!( ) const;
		bool done( ) const;
		bool disabled( ) const;

		CHEAT_ENUM_STRUCT_FILL(service_state2, unset)
	};

	class service_base2: public utl::noncopyable
	{
	public:
		virtual ~service_base2( );
		virtual utl::string_view name( ) const = 0;

		service_base2( ) = default;

		service_base2(service_base2&& other) noexcept;
		void operator=(service_base2&& other) noexcept;

		service_state2 state( ) const;
		void load( );

	protected:
		/**
		 * \brief
		 * \return true: loaded, false: skipped.
		 * throw exception if something wrong!
		 */
		virtual bool Do_load( ) = 0;

		virtual void On_load( ) { constexpr auto _ = 0; }
#if defined BOOST_THREAD_PROVIDES_INTERRUPTIONS
		virtual void On_stop( ) { constexpr auto _ = 0; }
#endif
		virtual void On_error( ) { constexpr auto _ = 0; }

		void Loading_access_assert( ) const;
	private:
		utl::atomic<service_state2> state__;
	};

	template <typename T>
	class service: public service_base2, public utl::one_instance_shared<T>
	{
	public:
		utl::string_view name( ) const final
		{
			return _Type_name<T>(false);
		}
	};
}

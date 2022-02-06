module;

#include "basic_includes.h"
#include <nstd/type_traits.h>

export module cheat.service:basic;

template<bool Unwrap, typename T>
auto _Ref_view(const std::span<T>& rng)
{
	if constexpr (!Unwrap)
	{
		return rng;
	}
	else
	{
		return std::views::transform(rng, []<class C>(C & item)->nstd::add_const_if<C, decltype(*item)>
		{
			return *item;
		});
	}
}

export namespace cheat
{
	enum class service_state : uint8_t
	{
		unset = 0
		, waiting
		, loading
		, loaded
		, error
	};

	class __declspec(novtable) basic_service
	{
	public:
		using executor = cppcoro::static_thread_pool;
		using task_type = cppcoro::task<bool>;
		using mutex_type = cppcoro::async_mutex;
		using value_type = std::shared_ptr<basic_service>;
		using deps_storage = std::vector<value_type>;

		template<class T>
		static value_type _Create( )
		{
			return std::make_shared<T>( );
		}

		basic_service( );
		virtual ~basic_service( );

		basic_service(const basic_service& other) = delete;
		basic_service(basic_service&& other) noexcept;
		basic_service& operator=(const basic_service& other) = delete;
		basic_service& operator=(basic_service&& other) noexcept;

		size_t _Add_dependency(value_type&& srv);
	public:

		virtual std::string_view name( ) const = 0;
		virtual const std::type_info& type( ) const = 0;

		struct sync_start { };

		service_state state( ) const;
		[[nodiscard]] task_type start(executor& ex) noexcept;
		[[nodiscard]] task_type::value_type start(executor& ex, sync_start) noexcept;
		[[nodiscard]] task_type::value_type start( ) noexcept;

	protected:
		void set_state(service_state state);

		//delayed constructor
		virtual void construct( )noexcept = 0;
		virtual bool load( ) noexcept
		{
			return true;
		}

	public:
		template<bool Unwrap>
		[[nodiscard]] auto _Deps( )
		{
			return _Ref_view<Unwrap, value_type>(deps_);
		}
		template<bool Unwrap>
		[[nodiscard]] auto _Deps( )const
		{
			return _Ref_view<Unwrap, const value_type>(deps_);
		}

	private:
		mutex_type lock_;
		deps_storage deps_;
		service_state state_ = service_state::unset;
	};
}



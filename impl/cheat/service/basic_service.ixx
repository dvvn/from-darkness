module;

#include <nstd/type name.h>

#include "includes.h"

export module cheat.service:basic;

_INLINE_VAR constexpr auto _Unwrap_smart_ptr = []<typename T>(T && ptr)->auto&
{
	if constexpr (std::is_const_v<T>)
	{
		const auto& tmp = *ptr;
		return tmp;
	}
	else
	{
		return *ptr;
	}
};

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
		using load_result = cppcoro::task<bool>;
		using mutex_type = cppcoro::async_mutex;
		using value_type = std::shared_ptr<basic_service>;
		using deps_storage = std::vector<value_type>;

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

		service_state state( ) const;
		//rename to 'do load'
		load_result load(executor& ex) noexcept;

	protected:
		void set_state(service_state state);

		//rename to 'construct' 'lazy load' or something like it
		virtual void load_async( )noexcept = 0;
		//rename to 'load'
		virtual bool load_impl( ) noexcept
		{
			return true;
		}

	public:
		template<bool Raw>
		auto _Deps( )
		{
			std::span<value_type> deps = deps_;
			if constexpr (Raw)
				return deps | std::views::transform(_Unwrap_smart_ptr);
			else
				return deps;
		}
		template<bool Raw>
		auto _Deps( )const
		{
			std::span<const value_type> deps = deps_;
			if constexpr (Raw)
				return deps | std::views::transform(_Unwrap_smart_ptr);
			else
				return deps;
		}

	private:
		mutex_type lock_;
		deps_storage deps_;
		service_state state_ = service_state::unset;
	};

#if 0
	template </*root_service*/class T>
	class shared_service
	{
	public:
		using element_type = T;
		using shared_type = stored_service<T>;

		shared_service(const shared_service& other) = delete;
		shared_service& operator=(const shared_service& other) = delete;

		shared_service(shared_service&& other) noexcept
		{
			obj_ = std::move(other.obj_);
#ifndef _DEBUG
			other.obj_ = nullptr;
#endif
		}

		shared_service& operator=(shared_service&& other) noexcept
		{
			std::swap(obj_, other.obj_);
			return *this;
		}

		template <typename ...Args>
		shared_service(Args&&...args)
		{
			auto obj = std::make_shared<T>(std::forward<Args>(args)...);
			obj_ = obj
#ifndef _DEBUG
				.get( )
#endif
				;
			add_to_loader(std::move(obj));
		}

		~shared_service( )
		{
#ifdef _DEBUG
			if (obj_.expired( ))
				return;
#endif
			runtime_assert(get_from_loader(type( )) == nullptr, "shared service destroyed before loader!");
		}

		static const std::type_info& type( ) { return typeid(T); }

		_NODISCARD shared_type share( ) const
		{
			auto stored = get_from_loader(type( ));
			runtime_assert(stored != nullptr, "unable to share service!");
			return std::dynamic_pointer_cast<T>(*stored);
		}

	private:
		_NODISCARD
#ifdef _DEBUG
			shared_type _Get( ) const { return shared_type(obj_); }
#else
			T* _Get( ) const noexcept { return obj_; }
#endif

	public:
		auto operator->( ) const { return _Get( ); }
		[[deprecated]]
		_NODISCARD T& operator*( ) const { return *_Get( ); }

	private:
#ifdef _DEBUG
		std::weak_ptr<T>
#else
		T*
#endif
			obj_;
	};
#endif
}



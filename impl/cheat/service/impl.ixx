module;

#include <nstd/runtime_assert_fwd.h>
#include <nstd/type name.h>

#include <cppcoro/static_thread_pool.hpp>
#include <cppcoro/task.hpp>
#include <cppcoro/async_mutex.hpp>

#include <string>
#include <functional>
#include <vector>

export module cheat.core.service;

constexpr auto _Fix_service_name(const std::string_view& name)
{
	auto str = nstd::drop_namespace(name, "cheat");
	constexpr auto dummy = std::string_view("_impl");
	if (str.ends_with(dummy))
		str.erase(str.end( ) - dummy.size( ), str.end( ));
	return str;
}

template <typename T>
_INLINE_VAR constexpr auto service_name = []
{
	constexpr auto raw = nstd::type_name<T>;
	const auto raw_str = _Fix_service_name(raw);
#if 1
	constexpr auto buff_size = _Fix_service_name(raw).size( );
	auto buff = nstd::string_to_buffer<buff_size>(raw_str);
#else
	auto buff = nstd::string_to_buffer<raw_str.size( )>(raw_str);
#endif
	return buff;
}();

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

	class basic_service;

	template <class T>
	struct stored_service : std::shared_ptr<T>
	{
		stored_service( ) = default;

		template <class Q>
		stored_service(Q&& service)
			: std::shared_ptr<T>(std::forward<Q>(service))
		{
			static_assert(std::derived_from<T, basic_service>);
		}

		/*template <class Q>
		stored_service(Q&& cast_service, stored_service_cast_tag)
			: std::shared_ptr<T>(std::forward<Q>(cast_service))
		{
		}*/
	};

	template <typename T>
	concept root_service = std::derived_from<T, basic_service>;

	template <root_service T>
	class service_shared;

	class __declspec(novtable) basic_service
	{
	public:
		using executor = cppcoro::static_thread_pool;
		using load_result = cppcoro::task<bool>;
		using value_type = stored_service<basic_service>;

		basic_service( );
		virtual ~basic_service( );

		basic_service(const basic_service& other) = delete;
		basic_service(basic_service&& other) noexcept;
		basic_service& operator=(const basic_service& other) = delete;
		basic_service& operator=(basic_service&& other) noexcept;

		const value_type* find(const std::type_info& info) const;
		value_type* find(const std::type_info& info);
		void erase(const std::type_info& info);
		using erase_pred = std::function<bool(const value_type&)>;
		void erase(const erase_pred& fn);
		void erase_all(const erase_pred& fn);
		void unload( );

		void add_dependency(value_type&& srv);

		template <root_service T>
		void add_dependency(const service_shared<T>& srv) { add_dependency(srv.share( )); }

		virtual std::string_view name( ) const = 0;
		virtual const std::type_info& type( ) const = 0;

		service_state state( ) const;
		load_result load(executor& ex) noexcept;

		virtual bool root_class( ) const = 0;

	protected:
		virtual load_result load_impl( ) noexcept = 0;

	private:
		cppcoro::async_mutex lock_;
		std::vector<stored_service<basic_service>> deps_;
		service_state state_ = service_state::unset;
	};

	class basic_service_shared
	{
	protected:
		using value_type = stored_service<basic_service>;

		void add_to_loader(value_type&& srv) const;
		value_type* get_from_loader(const std::type_info& info) const;
	};

	template <root_service T>
	class service_shared : public basic_service_shared
	{
	public:
		using element_type = T;
		using shared_type = stored_service<T>;

		service_shared(const service_shared& other) = delete;
		service_shared& operator=(const service_shared& other) = delete;

		service_shared(service_shared&& other) noexcept
		{
			obj_ = std::move(other.obj_);
#ifndef _DEBUG
			other.obj_ = nullptr;
#endif
	}

		service_shared& operator=(service_shared&& other) noexcept
		{
			std::swap(obj_, other.obj_);
			return *this;
		}

		template <typename ...Args>
		service_shared(Args&&...args)
		{
			auto obj = std::make_shared<T>(std::forward<Args>(args)...);
			obj_ = obj
#ifndef _DEBUG
				.get( )
#endif
				;
			add_to_loader(std::move(obj));
		}

		~service_shared( )
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
		_NODISCARD T& operator*( ) const { return *_Get( ); }

	private:
#ifdef _DEBUG
		std::weak_ptr<T>
#else
		T*
#endif
			obj_;
};

	template <typename T, bool Root = false>
	struct service : basic_service
	{
		std::string_view name( ) const final
		{
			return service_name<T>.view( );
			/*constexpr auto tmp   = nstd::type_name<T, "cheat">;
			constexpr auto dummy = std::string_view("_impl");
			if constexpr (tmp.ends_with(dummy))
				return tmp.substr(0, tmp.size( ) - dummy.size( ));
			else
				return tmp;*/
		}

		const std::type_info& type( ) const final
		{
			return typeid(T);
		}

		bool root_class( ) const final
		{
			return Root;
		}
	};
}
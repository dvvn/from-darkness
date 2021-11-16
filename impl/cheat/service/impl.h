#pragma once

#include "stored.h"

#include <nstd/runtime_assert_fwd.h>

#include <concepts>

#if defined(_DEBUG) || defined(CHEAT_HAVE_CONSOLE)
#define CEHAT_SERVICE_HAVE_NAME
#endif

namespace cppcoro
{
	class static_thread_pool;
	template <typename T>
	class [[nodiscard]] task;
}

namespace std
{
	template <class _Elem>
	struct char_traits;
	template <class _Elem, class _Traits>
	class basic_string_view;
	using string_view = basic_string_view<char, char_traits<char>>;
	template <class _Fty>
	class function;
}

namespace cheat
{
	struct basic_service_data;
	class basic_service;

	enum class service_state : uint8_t;

	//--

	template <typename T>
	concept root_service = std::derived_from<T, basic_service>;

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

		service_shared(const service_shared& other)            = delete;
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
			obj_     = obj
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

#ifdef CEHAT_SERVICE_HAVE_NAME
		virtual std::string_view name( ) const = 0;
#endif
		virtual const std::type_info& type( ) const = 0;

		service_state state( ) const;
		load_result load(executor& ex) noexcept;

		virtual bool root_class( ) const =0;

	protected:
		virtual load_result load_impl( ) noexcept = 0;
	private:
		struct impl;
		std::unique_ptr<impl> impl_;
	};
}

//for resharper
#define TODO_IMPLEMENT_ME \
	(void)this;\
	static_assert(false,__FUNCSIG__": not implemented!");\
	[]{}

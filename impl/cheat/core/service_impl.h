#pragma once

#include "service_state.h"
#include "stored_service.h"

#include <nstd/runtime_assert_fwd.h>
#include <cppcoro/task.hpp>
#include <concepts>

namespace cppcoro
{
	class static_thread_pool;
}

#ifndef _VECTOR_
namespace std
{
	template <class _Myvec>
	// ReSharper disable once CppInconsistentNaming
	class _Vector_iterator;
}
#endif

namespace cheat
{
	struct basic_service_data;
	class basic_service;

	template <typename T>
	concept root_service = std::derived_from<T, basic_service>;

	class basic_service_shared
	{
	protected:
		void add_to_loader(stored_service<>&& srv) const;
		stored_service<>* get_from_loader(const std::type_info& info) const;
	};

	template <root_service T>
	class service_shared : public basic_service_shared
	{
	public:
		using element_type = T;
		using shared_type = std::shared_ptr<T>;

		service_shared(const service_shared& other)            = delete;
		service_shared& operator=(const service_shared& other) = delete;

		service_shared(service_shared&& other) noexcept
		{
			obj_ = std::move(other.obj_);
#ifndef _DEBUG
			other.obj_ = {};
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
			obj_     =
#ifdef _DEBUG
					obj
#else
					*obj
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
			T* _Get() const noexcept { return obj_.get(); }
#endif

	public:
		auto operator->( ) const { return _Get( ); }
		_NODISCARD T& operator*( ) const { return *_Get( ); }

	private:
#ifdef _DEBUG
		std::weak_ptr<T>
#else
		std::reference_wrapper<T>
#endif
		obj_;
	};

	class __declspec(novtable) basic_service
	{
	public:
		using executor = cppcoro::static_thread_pool;
		using load_result = cppcoro::task<bool>;

		basic_service( );
		virtual ~basic_service( );

		basic_service(const basic_service& other) = delete;
		basic_service(basic_service&& other) noexcept;
		basic_service& operator=(const basic_service& other) = delete;
		basic_service& operator=(basic_service&& other) noexcept;

		const stored_service<>* find_service(const std::type_info& info) const;
		stored_service<>* find_service(const std::type_info& info);

		void remove_service(const std::type_info& info);

		void unload( );

		void add_dependency(stored_service<>&& srv);

		template <root_service T>
		void add_dependency(const service_shared<T>& srv) { add_dependency(srv.share( )); }

		service_state state( ) const;

		virtual std::string_view name( ) const = 0;
		virtual const std::type_info& type( ) const = 0;

		load_result load(executor& ex) noexcept;

	protected:
		virtual load_result load_impl( ) noexcept = 0;

	private:
		friend class services_loader;

		struct lock_impl;
		std::unique_ptr<lock_impl> lock_;
		std::unique_ptr<basic_service_data> deps_;
		service_state state_;
	};
}

//for resharper
#define TODO_IMPLEMENT_ME \
	(void)this;\
	static_assert(false,__FUNCSIG__": not implemented!");\
	[]{}

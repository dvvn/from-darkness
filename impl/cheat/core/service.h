#pragma once

//#include "detour hook/hook_utils.h"

#include <nstd/one_instance.h>
#include <nstd/type name.h>

#include <cppcoro/async_mutex.hpp>
#include <cppcoro/static_thread_pool.hpp>
#include <cppcoro/task.hpp>

#include <concepts>
#include <span>
#include <vector>

namespace cheat
{
#if defined(CHEAT_GUI_TEST) || defined(CHEAT_NETVARS_UPDATING)
#define CHEAT_MODE_INGAME 0
#else
#define CHEAT_MODE_INGAME 1
#endif

	enum class service_state : uint8_t
	{
		unset = 0
	  , waiting
	  , loading
	  , loaded
	  , error
	};

	class service_impl;

	template <typename T>
	concept root_service = std::derived_from<T, service_impl>;

	template <typename T>
	concept derived_service = !requires { T::get_ptr( ); } && root_service<T> && std::default_initializable<T> ;

	template <typename T>
	concept shared_service = root_service<T> && nstd::is_one_instance_shared<T>;

	class __declspec(novtable) service_impl
	{
	public:
		using executor = cppcoro::static_thread_pool;
		using mutex_type = cppcoro::async_mutex;
		using load_result = cppcoro::task<bool>;

		static size_t _Services_count( );

		using stored_service = std::shared_ptr<service_impl>;

	protected:
		using deps_storage = std::vector<stored_service>;

	public:
		service_impl( );
		virtual ~service_impl( );

		service_impl(const service_impl& other) = delete;
		service_impl(service_impl&& other) noexcept;
		service_impl& operator=(const service_impl& other) = delete;
		service_impl& operator=(service_impl&& other) noexcept;

		std::span<const stored_service> services( ) const;

		stored_service find_service(const std::type_info& info) const;

		template <root_service T>
		auto find_service( ) const
		{
			const auto found = find_service(typeid(T));
			return std::dynamic_pointer_cast<T>(found);
		}

	protected:
		template <typename Itr>
		struct iterator_proxy : Itr
		{
			using base_type = Itr;

			template <class T>
				requires(std::constructible_from<Itr, T>)
			iterator_proxy(T&& itr, const deps_storage* storage)
				: Itr(std::forward<T>(itr)), storage_(storage)
			{
			}

			bool valid( ) const { return *this != storage_->end( ); }

		private:
			const deps_storage* storage_;
		};

		iterator_proxy<deps_storage::iterator> find_service_itr(const std::type_info& info);
		iterator_proxy<deps_storage::const_iterator> find_service_itr(const std::type_info& info) const;

		template <root_service T>
		auto find_service_itr( ) { return find_service_itr(typeid(T)); }

		template <root_service T>
		auto find_service_itr( ) const { return find_service_itr(typeid(T)); }

	public:
		void remove_service(const std::type_info& info);

		template <root_service T>
		void remove_service( ) { remove_service(typeid(T)); }

		void unload( );

	private:
		stored_service& add_service_dependency(stored_service&& srv, const std::type_info& info);
		stored_service& add_service_dependency(const stored_service& srv, const std::type_info& info);

	public:
		template <derived_service T>
		auto wait_for_service( )
		{
			auto out = std::make_shared<T>( );
			add_service_dependency(out, typeid(T));
			return out;
		}

		template <shared_service T>
		void wait_for_service(bool steal = false)
		{
			add_service_dependency(T::get_ptr_shared(steal), typeid(T::value_type));
		}

		service_state state( ) const;
		virtual std::string_view name( ) const = 0;
		virtual const std::type_info& type( ) const = 0;
		virtual std::string_view debug_type( ) const = 0;
		virtual std::string_view debug_msg_loaded( ) const = 0;
		virtual std::string_view debug_msg_skipped( ) const = 0;
		virtual std::string_view debug_msg_error( ) const = 0;

		load_result load(executor& ex) noexcept;

	protected:
		virtual load_result load_impl( ) noexcept = 0;

	private:
		service_state state_ = service_state::unset;
		deps_storage deps_;
		mutex_type lock_;
	};

	template <typename T>
	struct service : service_impl
	{
		std::string_view name( ) const override { return nstd::type_name<T, "cheat">; }
		const std::type_info& type( ) const final { return typeid(T); }
		std::string_view debug_type( ) const override { return "Service"; }
		std::string_view debug_msg_loaded( ) const override { return "loaded"; }
		std::string_view debug_msg_skipped( ) const override { return "skipped"; }
		std::string_view debug_msg_error( ) const override { return "NOT loaded"; }
	};

	template <typename T>
	struct service_instance_shared : service<T>, nstd::one_instance_shared<T>
	{
	};

	namespace detail
	{
		enum class log_type
		{
			LOADED
		  , SKIPPED
			// ReSharper disable once CppInconsistentNaming
		  , ERROR_ //fuck ERROR macro 
		};

		std::string make_log_message(const service_impl* srv, log_type type, std::string_view extra = "");
	}
}

#define CHEAT_FUNC_MESSAGE(_MSG_) \
	__pragma(message(__FUNCTION__": "##_MSG_))

#define CHEAT_SERVICE_RESULT(msg, ret)\
	using cheat::detail::log_type;\
	using cheat::detail::make_log_message;\
	CHEAT_CONSOLE_LOG(msg);\
	co_return (ret);

#define CHEAT_SERVICE_LOADED \
{\
	CHEAT_SERVICE_RESULT(make_log_message(this, log_type::LOADED), true)\
}

#define CHEAT_SERVICE_SKIPPED \
{\
	CHEAT_FUNC_MESSAGE("unused");\
	CHEAT_SERVICE_RESULT(make_log_message(this, log_type::SKIPPED), true)\
}

#define CHEAT_SERVICE_NOT_LOADED(why) \
{\
	runtime_assert(why);\
	CHEAT_SERVICE_RESULT(make_log_message(this, log_type::ERROR_, #why), false)\
}

#define CHEAT_SERVICE_INIT_1 CHEAT_SERVICE_LOADED
#define CHEAT_SERVICE_INIT_0 CHEAT_SERVICE_SKIPPED

#define CHEAT_SERVICE_INIT(FT) \
	_CONCAT(CHEAT_SERVICE_INIT_,FT)
#define CHEAT_CALL_BLOCKER\
	runtime_assert("Unused but called");\
	CHEAT_FUNC_MESSAGE("disabled");\
	(void)this;

//for resharper
#define TODO_IMPLEMENT_ME \
	(void)this;\
	static_assert(false,__FUNCTION__": not implemented!");\
	[]{}

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
	#define CHEAT_SERVICE_INGAME 0
#else
	#define CHEAT_SERVICE_INGAME 1
#endif

	enum class service_state : uint8_t
	{
		unset = 0
	  , waiting
	  , loading
	  , loaded
	  , error
	};

	class service_base;

	template <typename T>
	concept root_service = std::derived_from<T, service_base>;

	template <typename T>
	concept derived_service = !requires { T::get_ptr( ); } && root_service<T> && std::default_initializable<T> ;

	template <typename T>
	concept shared_service = root_service<T> && nstd::is_one_instance_shared<T>;

	class service_base
	{
	public:
		using load_result = cppcoro::task<bool>;
		using mutex_type = cppcoro::async_mutex;

		using executor = cppcoro::static_thread_pool;

		using stored_service = std::shared_ptr<service_base>;

		service_base() = default;
		virtual ~service_base();

		service_base(const service_base& other) = delete;
		service_base(service_base&& other) noexcept;
		service_base& operator=(const service_base& other) = delete;
		service_base& operator=(service_base&& other) noexcept;

		virtual std::string_view name() const = 0;
		virtual const std::type_info& type() const = 0;

		std::span<const stored_service> services() const;

		stored_service find_service(const std::type_info& info) const;

		template <root_service T>
		auto find_service() const
		{
			const auto found = find_service(typeid(T));
			return std::dynamic_pointer_cast<T>(found);
		}

	private:
		void add_service_dependency(stored_service&& srv, const std::type_info& info);

	public:
		template <derived_service T>
		void wait_for_service()
		{
			add_service_dependency(std::make_shared<T>( ), typeid(T));
		}

		template <shared_service T>
		void wait_for_service(bool steal = false)
		{
			add_service_dependency(T::get_ptr_shared(steal), typeid(T::value_type));
		}

		service_state state() const;

		virtual void reset();

		virtual load_result load(executor& ex) noexcept;

	protected:
		virtual load_result load_impl() noexcept = 0;

	private:
		service_state state_ = service_state::unset;
		std::vector<stored_service> deps_;
		mutex_type lock_;
	};

	template <typename T>
	struct service : service_base, nstd::one_instance_shared<T>
	{
		std::string_view name() const override { return nstd::type_name<T, "cheat">; }
		const std::type_info& type() const final { return typeid(T); }
	};

	#define CHEAT_SERVICE_RESULT(msg, ret)\
	CHEAT_CONSOLE_LOG(msg);\
	co_return ret;

#define CHEAT_SERVICE_LOADED\
	{CHEAT_SERVICE_RESULT(std::format("Service \'{}\' loaded!",this->name()), true)}
#define CHEAT_SERVICE_SKIPPED\
	{CHEAT_SERVICE_RESULT(std::format("Service \'{}\' skipped!",this->name()), true)}
#define CHEAT_SERVICE_NOT_LOADED(why,...)\
	{\
	__VA_ARGS__\
	runtime_assert(why);\
	CHEAT_SERVICE_RESULT(std::format("Service \'{}\' NOT loaded! "##why,this->name()), false)\
	}
}

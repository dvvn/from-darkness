#pragma once

#if !(defined _M_IX86) && !(defined _M_X64) && !(defined __i386__) && !(defined __x86_64__)
#error MinHook supports only x86 and x64 windows systems.
#endif

#ifndef  __cplusplus
#error C++ compiler required.
#endif

#ifndef _INC_DDEMLH
#define _INC_DDEMLH
#endif

//#include "vtable_counter.h"
#include "trampoline.h"

#include "cheat/utils/one_instance.h"

namespace cheat::utl::hooks
{
	enum class STATUS:size_t
	{
		/// @brief Unknown error. Should not be returned.
		UNKNOWN = 0,

		/// @brief Successful.
		OK,

		/// @brief The hook for the specified target function is already created.
		ERROR_ALREADY_CREATED,

		/// @brief The hook for the specified target function is not created yet.
		ERROR_NOT_CREATED,

		/// @brief The hook for the specified target function is already enabled.
		ERROR_ENABLED,

		/// @brief The hook for the specified target function is not enabled yet, or already disabled.
		ERROR_DISABLED,

		/// @brief The specified pointer is invalid. It points the address of non-allocated and/or non-executable region.
		ERROR_NOT_EXECUTABLE,

		/// @brief The specified pointer is invalid. It points the address of non-allocated and/or non-readable region.
		ERROR_NOT_READABLE,

		/// @brief The specified target function cannot be hooked.
		ERROR_UNSUPPORTED_FUNCTION,

		/// @brief Failed to allocate mem.
		ERROR_MEMORY_ALLOC,

		/// @brief Failed to change the mem protection.

		ERROR_MEMORY_PROTECT,

		/// @brief The specified module is not loaded.
		ERROR_MODULE_NOT_FOUND,

		/// @brief The specified class is not virtual.
		ERROR_VTABLE_NOT_FOUND,

		/// @brief The specified function is not found.
		ERROR_FUNCTION_NOT_FOUND,

		COUNT
	};

	namespace detail
	{
		struct status_ex_impl
		{
			status_ex_impl(STATUS s) : status__(s)
			{
			}

			operator STATUS( ) const
			{
				return status__;
			}

			bool operator==(const status_ex_impl&) const = default;

		private:
			STATUS status__;
		};
	}

	class hook_entry final: public detail::trampoline
	{
	public:
		~hook_entry( ) override;
		hook_entry( ) = default;

		hook_entry(const hook_entry&) = delete;
		hook_entry& operator=(const hook_entry&) = delete;
		hook_entry(hook_entry&& other) noexcept;
		hook_entry& operator=(hook_entry&& other) noexcept;

		STATUS set_state(bool enable);

		ips_type backup; // Original prologue of the target function.
		bool     enabled = false;
	};

	namespace detail
	{
		struct hook_result_status
		{
			hook_result_status(STATUS s): status(s)
			{
			}

			bool   operator==(const hook_result_status&) const = default;
			STATUS status;
		};
	}

	struct hook_result: detail::hook_result_status
	{
		hook_result(STATUS status) : hook_result_status(status)
		{
		}

		hook_entry* entry = nullptr;
	};

	class context_base
	{
	protected:
		virtual ~context_base( ) = default;
	public:
		virtual hook_result create_hook(LPVOID target, LPVOID detour) =0;
		virtual STATUS      remove_hook(LPVOID target, bool force = false) =0;
		virtual STATUS      enable_hook(LPVOID target) =0;
		virtual STATUS      disable_hook(LPVOID target) =0;
		virtual hook_result find_hook(LPVOID target) =0;
		virtual void        remove_all_hooks( ) =0;
		virtual STATUS      enable_all_hooks( ) =0;
		virtual STATUS      disable_all_hooks( ) =0;
	};

	class context final: public context_base
	{
	public:
		using storage_type = vector<hook_entry>;
		using element_type = storage_type::value_type;

		hook_result create_hook(LPVOID target, LPVOID detour) override;
		STATUS      remove_hook(LPVOID target, bool force) override;
		STATUS      enable_hook(LPVOID target) override;
		STATUS      disable_hook(LPVOID target) override;
		hook_result find_hook(LPVOID target) override;
		void        remove_all_hooks( ) override;
		STATUS      enable_all_hooks( ) override;
		STATUS      disable_all_hooks( ) override;

		//deprecated, threads mut be paused manually
		//bool pause_threads = true;

	private:
		hook_entry* Find_hook_internal_(LPVOID target);

		STATUS Set_state_all_(bool enable, bool ignore_errors = false);
		STATUS Reset_all_( );
		STATUS Set_hook_state_(LPVOID target, bool enable);

		storage_type storage__;
	};

	/// @brief Translates the STATUS to its name as a string.
	static string_view status_to_string(STATUS status);

	class context_safe final: public context_base
	{
	public:
		using lock_type = mutex;

		hook_result create_hook(LPVOID target, LPVOID detour) override;
		STATUS      remove_hook(LPVOID target, bool force) override;
		STATUS      enable_hook(LPVOID target) override;
		STATUS      disable_hook(LPVOID target) override;
		hook_result find_hook(LPVOID target) override;
		void        remove_all_hooks( ) override;
		STATUS      enable_all_hooks( ) override;
		STATUS      disable_all_hooks( ) override;

	private:
		context   ctx__;
		lock_type mtx__;
	};

	using context_shared = one_instance_shared<context_safe>;

#if 0
    /// @brief Creates a Hook for the specified Windows API function, in disabled state.
    /// @param pszModule A pointer to the loaded module name which contains the target function.
    /// @param pszProcName A pointer to the target function name, which will be overridden by the detour function.
    /// @param pDetour A pointer to the detour function, which will override the target function.
    /// @return
    [[deprecated("use CreateHook directly. result are same")]]
    auto create_hook_win_api(LPCWSTR pszModule, LPCSTR pszProcName, LPVOID pDetour) -> hook_result;
#endif
}

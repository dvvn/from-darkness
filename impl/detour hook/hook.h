#pragma once

//#include "vtable_counter.h"
#include "trampoline.h"

#include "nstd/one_instance.h"

namespace dhooks
{
	class hook_entry final: public detail::trampoline
	{
	public:
		~hook_entry( );
		hook_entry( ) = default;

		hook_entry(const hook_entry&)            = delete;
		hook_entry& operator=(const hook_entry&) = delete;
		hook_entry(hook_entry&& other) noexcept;
		hook_entry& operator=(hook_entry&& other) noexcept;

		hook_status set_state(bool enable);

		ips_type backup; // Original prologue of the target function.
		bool     enabled = false;
	};

	struct hook_result
	{
		template <std::convertible_to<hook_status> T>
		hook_result(T status)
			: status(status)
		{
			//runtime_assert(this->status == hook_status::OK);
		}

		hook_status status;
		hook_entry* entry = nullptr;
	};

	class context_base
	{
	public:
		virtual ~context_base( ) = default;

		virtual hook_result create_hook(LPVOID target, LPVOID detour) = 0;
		virtual hook_status remove_hook(LPVOID target, bool force = false) = 0;
		virtual hook_status enable_hook(LPVOID target) = 0;
		virtual hook_status disable_hook(LPVOID target) = 0;
		virtual hook_result find_hook(LPVOID target) = 0;
		virtual void        remove_all_hooks( ) = 0;
		virtual hook_status enable_all_hooks( ) = 0;
		virtual hook_status disable_all_hooks( ) = 0;
	};

	class context final: public context_base
	{
	public:
		using element_type = hook_entry;
		struct storage_type;

		context( );
		~context() override;

		hook_result create_hook(LPVOID target, LPVOID detour) override;
		hook_status remove_hook(LPVOID target, bool force) override;
		hook_status enable_hook(LPVOID target) override;
		hook_status disable_hook(LPVOID target) override;
		hook_result find_hook(LPVOID target) override;
		void        remove_all_hooks( ) override;
		hook_status enable_all_hooks( ) override;
		hook_status disable_all_hooks( ) override;

	private:
		std::unique_ptr<storage_type> storage_;
	};

	class context_safe final: public context_base
	{
	public:
		struct lock_type;

		context_safe( );

		hook_result create_hook(LPVOID target, LPVOID detour) override;
		hook_status remove_hook(LPVOID target, bool force) override;
		hook_status enable_hook(LPVOID target) override;
		hook_status disable_hook(LPVOID target) override;
		hook_result find_hook(LPVOID target) override;
		void        remove_all_hooks( ) override;
		hook_status enable_all_hooks( ) override;
		hook_status disable_all_hooks( ) override;

	private:
		context                    ctx_;
		std::unique_ptr<lock_type> mtx_;
	};

#if 0
	/// @brief Creates a Hook for the specified Windows API function, in disabled state.
	/// @param pszModule A pointer to the loaded module name which contains the target function.
	/// @param pszProcName A pointer to the target function name, which will be overridden by the detour function.
	/// @param pDetour A pointer to the detour function, which will override the target function.
	/// @return
	[[deprecated("use CreateHook directly. result are same")]]
	auto create_hook_win_api(LPCWSTR pszModule, LPCSTR pszProcName, LPVOID pDetour)->hook_result;
#endif
}

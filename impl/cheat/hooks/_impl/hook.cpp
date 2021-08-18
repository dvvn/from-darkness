#include "hook.h"

using namespace cheat;
using namespace hooks;
using namespace hooks::detail;
using namespace utl;
using namespace winapi;

_STD_BEGIN
	template < >
	struct hash<hook_entry>
	{
		size_t operator()(const hook_entry& val) const noexcept
		{
			return reinterpret_cast<size_t>(val.target);
		}
	};
_STD_END

#if 0

//i found it uselles

static DWORD_PTR FindOldIP(HOOK_ENTRY& pHook, DWORD_PTR ip)
{
	if(pHook.patchAbove && ip == ((DWORD_PTR)pHook.target - sizeof(JMP_REL)))
		return (DWORD_PTR)pHook.target;

	for(UINT i = 0; i < pHook.nIP; ++i)
	{
		if(ip == ((DWORD_PTR)pHook.pTrampoline.get( ) + pHook.newIPs[i]))
			return (DWORD_PTR)pHook.target + pHook.oldIPs[i];
	}

#if defined(_M_X64) || defined(__x86_64__)
	// Check relay function.
	if(ip == (DWORD_PTR)pHook.pDetour)
		return (DWORD_PTR)pHook.target;
#endif

	return 0;
}

static DWORD_PTR FindNewIP(HOOK_ENTRY& pHook, DWORD_PTR ip)
{
	for(UINT i = 0; i < pHook.nIP; ++i)
	{
		if(ip == ((DWORD_PTR)pHook.target + pHook.oldIPs[i]))
			return (DWORD_PTR)pHook.pTrampoline.get( ) + pHook.newIPs[i];
	}

	return 0;
}

static void ProcessThreadIPs(HANDLE hThread, const hooks_storage::iterator& pos, bool enable)
{
	// If the thread suspended in the overwritten area,
	// std::move IP to the proper address.

	CONTEXT c;
#if defined(_M_X64) || defined(__x86_64__)
	const auto pIP = &c.Rip;
#else
	const auto pIP = &c.Eip;
#endif

	c.ContextFlags = CONTEXT_CONTROL;
	if(!GetThreadContext(hThread, &c))
		return;

	for(auto pHook : storage::get( ).filter_enabled(std::span{pos, storage::get( ).end( )}, enable))
	{
		DWORD_PTR ip;

		if(enable)
			ip = FindNewIP(*pHook, *pIP);
		else
			ip = FindOldIP(*pHook, *pIP);

		if(ip != 0)
		{
			*pIP = ip;
			SetThreadContext(hThread, &c);
		}
	}
}
#endif

#ifdef _DEBUG_OFF
status_ex::status_ex(status s) : status_ex_impl{s}
{
	if(s == status::OK)
		return;

	const auto start = string_view("Error detected: status ");
	const auto end = status_to_string(s);

	std::string str;
	str.reserve(start.size( ) + end.size( ));

	str += start;
	str += end;

	throw std::runtime_error(std::move(str));
}
#endif

//--------

hook_entry::~hook_entry( )
{
	runtime_assert(!this->enabled, "Unable to destroy enabled hook entry!");
}

hook_entry::hook_entry(hook_entry&& other) noexcept
{
	*this = std::move(other);
}

hook_entry& hook_entry::operator=(hook_entry&& other) noexcept
{
	*static_cast<trampoline*>(this) = static_cast<trampoline&&>(other);

	backup  = std::move(other.backup);
	enabled = other.enabled;

	other.enabled = false;

	return *this;
}

hook_status hook_entry::set_state(bool enable)
{
	if (this->enabled == enable)
		return enable ? hook_status::ERROR_ENABLED : hook_status::ERROR_DISABLED;

	auto   patch_target = static_cast<LPBYTE>(this->target);
	SIZE_T patch_size   = sizeof(JMP_REL);

	if (this->patch_above)
	{
		patch_target -= sizeof(JMP_REL);
		patch_size += sizeof(JMP_REL_SHORT);
	}

	DWORD old_protect;
	if (!VirtualProtect(patch_target, patch_size, PAGE_EXECUTE_READWRITE, &old_protect))
		return hook_status::ERROR_MEMORY_PROTECT;

	if (enable)
	{
		auto again = false;
		(void)again;
	_TRY_AGAIN:
		__try
		{
			const auto jmp_rel = reinterpret_cast<JMP_REL*>(patch_target);
			jmp_rel->opcode    = 0xE9;
			jmp_rel->operand   = static_cast<UINT32>(static_cast<LPBYTE>(this->detour) - (patch_target + sizeof(JMP_REL)));
			if (this->patch_above)
			{
				const auto short_jmp = static_cast<JMP_REL_SHORT*>(this->target);
				short_jmp->opcode    = 0xEB;
				short_jmp->operand   = static_cast<UINT8>(0 - (sizeof(JMP_REL_SHORT) + sizeof(JMP_REL)));
			}
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			DWORD dummy;
			if (again || !VirtualProtect(patch_target, patch_size, PAGE_EXECUTE_READWRITE, &dummy))
			{
				VirtualProtect(patch_target, patch_size, old_protect, &dummy);
				return hook_status::ERROR_MEMORY_PROTECT;
			}
			again = true;
			goto _TRY_AGAIN;
		}
	}
	else
	{
		if (this->patch_above)
			memcpy(patch_target, this->backup.data( ), sizeof(JMP_REL) + sizeof(JMP_REL_SHORT));
		else
			memcpy(patch_target, this->backup.data( ), sizeof(JMP_REL));
	}

	VirtualProtect(patch_target, patch_size, old_protect, &old_protect);

	// Just-in-case measure.
	FlushInstructionCache(GetCurrentProcess( ), patch_target, patch_size);

	this->enabled = enable;

	return hook_status::OK;
}

hook_status context::Set_hook_state_(LPVOID target, bool enable)
{
	const auto entry = Find_hook_internal_(target);
	if (!entry)
		return hook_status::ERROR_NOT_CREATED;

	if (entry->enabled == enable)
		return enable ? hook_status::ERROR_ENABLED : hook_status::ERROR_DISABLED;

	/*const auto theads = frozen_threads_storage(pause_threads);
	(void)theads;*/
	return entry->set_state(enable);
}

hook_result context::create_hook(LPVOID target, LPVOID detour)
{
	if (!memory_block(target).executable( ) || !memory_block(detour).executable( ))
		return hook_status::ERROR_NOT_EXECUTABLE;

#if 0
	if(storage_.find(target) != nullptr)
		return hook_status::ERROR_ALREADY_CREATED;
	if(storage_.find(pDetour) != nullptr)
		return hook_status::ERROR_ALREADY_CREATED;
#endif

	if (target == detour)
		return hook_status::ERROR_UNSUPPORTED_FUNCTION;

	const auto check_ptr_helper = [&](LPVOID checked)
	{
		return checked == target || checked == detour;
	};
	for (const auto& value: storage__)
	{
		if (check_ptr_helper(value.target) || check_ptr_helper(value.detour))
			return hook_status::ERROR_ALREADY_CREATED;
	}

	auto ct = trampoline( );
	if (!ct.fix_page_protection( ))
		return hook_status::ERROR_MEMORY_PROTECT;

	ct.target = target;
	ct.detour = detour;
	if (!ct.create( ))
		return hook_status::ERROR_UNSUPPORTED_FUNCTION;

	auto& new_hook = storage__.emplace_back( );

	static_cast<trampoline&>(new_hook) = std::move(ct);

	new_hook.enabled = false;

#ifdef UTILS_X64
	new_hook.detour = ct.pRelay;
#endif
	// Back up the target function.

	if (new_hook.patch_above)
		memcpy(new_hook.backup.data( ), static_cast<LPBYTE>(target) - sizeof(JMP_REL), sizeof(JMP_REL) + sizeof(JMP_REL_SHORT));
	else
		memcpy(new_hook.backup.data( ), target, sizeof(JMP_REL));

	hook_result ret(hook_status::OK);
	ret.entry = &new_hook;

	return ret;
}

hook_status context::remove_hook(LPVOID target, bool force)
{
	const auto entry = Find_hook_internal_(target);
	if (!entry)
		return hook_status::ERROR_NOT_CREATED;

	if (entry->enabled)
	{
		if (const auto status = entry->set_state(false); status != hook_status::OK)
		{
			if (!force || status != hook_status::ERROR_MEMORY_PROTECT)
				return status;
		}
	}

	//erase if map, null if vector

	//storage_.erase(entry);
	*entry = { };

	return hook_status::OK;
}

hook_status context::enable_hook(LPVOID target)
{
	return Set_hook_state_(target, true);
}

hook_status context::disable_hook(LPVOID target)
{
	return Set_hook_state_(target, false);
}

hook_result context::find_hook(LPVOID target)
{
	const auto entry = Find_hook_internal_(target);
	if (!entry)
		return hook_status::ERROR_NOT_CREATED;

	hook_result result(hook_status::OK);
	result.entry = /*boost::addressof(entry->second)*/entry;
	return result;
}
#if 0
auto minhook::create_hook_win_api(LPCWSTR pszModule, LPCSTR pszProcName, LPVOID pDetour) -> hook_result
{
	const auto hModule = GetModuleHandleW(pszModule);
	if(hModule == nullptr)
		return hook_status::ERROR_MODULE_NOT_FOUND;

	const auto target = static_cast<LPVOID>(GetProcAddress(hModule, pszProcName));
	if(target == nullptr)
		return hook_status::ERROR_FUNCTION_NOT_FOUND;

	return create_hook(target, pDetour);
}
#endif
void context::remove_all_hooks( )
{
	this->Reset_all_( );
}

hook_status context::enable_all_hooks( )
{
	return this->Set_state_all_(true);
}

hook_status context::disable_all_hooks( )
{
	return this->Set_state_all_(false);
}

hook_entry* context::Find_hook_internal_(LPVOID target)
{
	if (target)
	{
		for (auto& h: storage__)
		{
			if (h.target == target)
				return std::addressof(h);
		}
	}

	return nullptr;
}

hook_status context::Set_state_all_(bool enable, bool ignore_errors)
{
	//auto frozen = frozen_threads_storage(false);

	auto storage_active = storage__ | ranges::views::filter([](const hook_entry& h)
	{
		return h.target != nullptr;
	});
	const auto begin = storage_active.begin( );
	const auto end   = storage_active.end( );

	for (auto itr_main = begin; itr_main != end; ++itr_main)
	{
		auto& value = *itr_main;
		if (value.enabled == enable)
			continue;

#if 0
		if(pause_threads)
		{
			//fill only if any hook enabled
			frozen.fill( );
		}
#endif

#ifdef _DEBUG
		hook_status main_status = hook_status::UNKNOWN;
		try
		{
			main_status = value.set_state(enable);
			if (main_status == hook_status::OK)
				runtime_assert("Error in code");
			continue;
		}
		catch ([[maybe_unused]] const std::runtime_error& e)
		{
			if (ignore_errors)
				continue;
		}
#else
		const auto main_status = value.set_state(enable);
		if(main_status == hook_status::OK)
			continue;
		if(ignore_errors)
			continue;
#endif

		//restore changes back
		enable = !enable;
		for (auto itr_child = begin; itr_child != itr_main; ++itr_child)
		{
			auto& value_child = *itr_child;
			if (value_child.enabled == enable)
				continue;
			if (const auto temp_status = value_child.set_state(enable); temp_status != hook_status::OK)
			{
				runtime_assert("Unable to revert hook state!");
				return temp_status;
			}
		}
		return main_status;
	}

	return hook_status::OK;
}

hook_status context::Reset_all_( )
{
	const auto result = Set_state_all_(false, true);
	storage__.clear( );
	return result;
}

template <typename ...Args>
static auto _Lock_and_work(context_safe::lock_type& mtx, Args&&...args)
{
	const auto lock = std::scoped_lock(mtx);
	return std::invoke(std::forward<Args>(args)...);
}

#define LOCK_AND_WORK(fn,...)\
	_Lock_and_work(mtx__, &context_base::##fn, ctx__,## __VA_ARGS__)

hook_result context_safe::create_hook(LPVOID target, LPVOID detour)
{
	return LOCK_AND_WORK(create_hook, target, detour);
}

hook_status context_safe::remove_hook(LPVOID target, bool force)
{
	return LOCK_AND_WORK(remove_hook, target, force);
}

hook_status context_safe::enable_hook(LPVOID target)
{
	return LOCK_AND_WORK(enable_hook, target);
}

hook_status context_safe::disable_hook(LPVOID target)
{
	return LOCK_AND_WORK(disable_hook, target);
}

hook_result context_safe::find_hook(LPVOID target)
{
	return LOCK_AND_WORK(find_hook, target);
}

void context_safe::remove_all_hooks( )
{
	LOCK_AND_WORK(remove_all_hooks);
}

hook_status context_safe::enable_all_hooks( )
{
	return LOCK_AND_WORK(enable_all_hooks);
}

hook_status context_safe::disable_all_hooks( )
{
	return LOCK_AND_WORK(disable_all_hooks);
}

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
    if (pHook.patchAbove && ip == ((DWORD_PTR)pHook.target - sizeof(JMP_REL)))
        return (DWORD_PTR)pHook.target;

    for (UINT i = 0; i < pHook.nIP; ++i)
    {
        if (ip == ((DWORD_PTR)pHook.pTrampoline.get() + pHook.newIPs[i]))
            return (DWORD_PTR)pHook.target + pHook.oldIPs[i];
    }

#if defined(_M_X64) || defined(__x86_64__)
    // Check relay function.
    if (ip == (DWORD_PTR)pHook.pDetour)
        return (DWORD_PTR)pHook.target;
#endif

    return 0;
}

static DWORD_PTR FindNewIP(HOOK_ENTRY& pHook, DWORD_PTR ip)
{
    for (UINT i = 0; i < pHook.nIP; ++i)
    {
        if (ip == ((DWORD_PTR)pHook.target + pHook.oldIPs[i]))
            return (DWORD_PTR)pHook.pTrampoline.get() + pHook.newIPs[i];
    }

    return 0;
}

static void ProcessThreadIPs(HANDLE hThread, const hooks_storage::iterator& pos, bool enable)
{
    // If the thread suspended in the overwritten area,
    // move IP to the proper address.

    CONTEXT c;
#if defined(_M_X64) || defined(__x86_64__)
    const auto pIP = &c.Rip;
#else
    const auto pIP = &c.Eip;
#endif

    c.ContextFlags = CONTEXT_CONTROL;
    if (!GetThreadContext(hThread, &c))
        return;

    for (auto pHook : storage::get().filter_enabled(std::span{ pos, storage::get().end() }, enable))
    {
        DWORD_PTR ip;

        if (enable)
            ip = FindNewIP(*pHook, *pIP);
        else
            ip = FindOldIP(*pHook, *pIP);

        if (ip != 0)
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
    if (s == status::OK)
        return;

    const auto start = string_view("Error detected: status ");
    const auto end = status_to_string(s);

    std::string str;
    str.reserve(start.size( ) + end.size( ));

    str += start;
    str += end;

    throw std::runtime_error(move(str));
}
#endif

//--------

hook_entry::~hook_entry( )
{
	BOOST_ASSERT_MSG(!this->enabled, "Unable to destroy enabled hook entry!");
}

hook_entry::hook_entry(hook_entry&& other) noexcept
{
	*this = move(other);
}

hook_entry& hook_entry::operator=(hook_entry&& other) noexcept
{
	*static_cast<trampoline*>(this) = static_cast<trampoline&&>(other);

	backup = move(other.backup);
	enabled = other.enabled;

	other.enabled = false;

	return *this;
}

STATUS hook_entry::set_state(bool enable)
{
	if (this->enabled == enable)
		return enable ? STATUS::ERROR_ENABLED : STATUS::ERROR_DISABLED;

	auto   patch_target = static_cast<LPBYTE>(this->target);
	SIZE_T patch_size = sizeof(JMP_REL);

	if (this->patch_above)
	{
		patch_target -= sizeof(JMP_REL);
		patch_size += sizeof(JMP_REL_SHORT);
	}

	DWORD old_protect;
	if (!VirtualProtect(patch_target, patch_size, PAGE_EXECUTE_READWRITE, &old_protect))
		return STATUS::ERROR_MEMORY_PROTECT;

	if (enable)
	{
		auto again = false;
		(void)again;
	_TRY_AGAIN:
		__try
		{
			const auto jmp_rel = reinterpret_cast<JMP_REL*>(patch_target);
			jmp_rel->opcode = 0xE9;
			jmp_rel->operand = static_cast<UINT32>(static_cast<LPBYTE>(this->detour) - (patch_target + sizeof(JMP_REL)));
			if (this->patch_above)
			{
				const auto short_jmp = static_cast<JMP_REL_SHORT*>(this->target);
				short_jmp->opcode = 0xEB;
				short_jmp->operand = static_cast<UINT8>(0 - (sizeof(JMP_REL_SHORT) + sizeof(JMP_REL)));
			}
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			DWORD dummy;
			if (again || !VirtualProtect(patch_target, patch_size, PAGE_EXECUTE_READWRITE, &dummy))
			{
				VirtualProtect(patch_target, patch_size, old_protect, &dummy);
				return STATUS::ERROR_MEMORY_PROTECT;
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

	return STATUS::OK;
}

STATUS context::Set_hook_state_(LPVOID target, bool enable)
{
	const auto entry = Find_hook_internal_(target);
	if (!entry)
		return STATUS::ERROR_NOT_CREATED;

	if (entry->enabled == enable)
		return enable ? STATUS::ERROR_ENABLED : STATUS::ERROR_DISABLED;

	/*const auto theads = frozen_threads_storage(pause_threads);
	(void)theads;*/
	return entry->set_state(enable);
}

hook_result context::create_hook(LPVOID target, LPVOID detour)
{
	if (!_Is_address_executable(target) || !_Is_address_executable(detour))
		return STATUS::ERROR_NOT_EXECUTABLE;

#if 0
    if (storage_.find(target) != nullptr)
        return STATUS::ERROR_ALREADY_CREATED;
    if (storage_.find(pDetour) != nullptr)
        return STATUS::ERROR_ALREADY_CREATED;
#endif

	if (target == detour)
		return STATUS::ERROR_UNSUPPORTED_FUNCTION;

	const auto check_ptr_helper = [&](LPVOID checked)
	{
		return checked == target || checked == detour;
	};
	for (auto& value: storage__)
	{
		if (check_ptr_helper(value.target) || check_ptr_helper(value.detour))
			return STATUS::ERROR_ALREADY_CREATED;
	}

	auto ct = trampoline( );
	if (!ct.fix_page_protection( ))
		return STATUS::ERROR_MEMORY_PROTECT;

	ct.target = target;
	ct.detour = detour;
	if (!ct.create( ))
		return STATUS::ERROR_UNSUPPORTED_FUNCTION;

	auto& new_hook = storage__.emplace_back( );

	static_cast<trampoline&>(new_hook) = move(ct);

	new_hook.enabled = false;

#ifdef UTILS_X64
    new_hook.detour = ct.pRelay;
#endif
	// Back up the target function.

	if (new_hook.patch_above)
		memcpy(new_hook.backup.data( ), static_cast<LPBYTE>(target) - sizeof(JMP_REL), sizeof(JMP_REL) + sizeof(JMP_REL_SHORT));
	else
		memcpy(new_hook.backup.data( ), target, sizeof(JMP_REL));

	hook_result ret(STATUS::OK);
	ret.entry = &new_hook;

	return ret;
}

STATUS context::remove_hook(LPVOID target, bool force)
{
	const auto entry = Find_hook_internal_(target);
	if (!entry)
		return STATUS::ERROR_NOT_CREATED;

	if (entry->enabled)
	{
		if (const auto status = entry->set_state(false); status != STATUS::OK)
		{
			if (!force || status != STATUS::ERROR_MEMORY_PROTECT)
				return status;
		}
	}

	//erase if map, null if vector

	//storage_.erase(entry);
	*entry = { };

	return STATUS::OK;
}

STATUS context::enable_hook(LPVOID target)
{
	return Set_hook_state_(target, true);
}

STATUS context::disable_hook(LPVOID target)
{
	return Set_hook_state_(target, false);
}

hook_result context::find_hook(LPVOID target)
{
	const auto entry = Find_hook_internal_(target);
	if (!entry)
		return STATUS::ERROR_NOT_CREATED;

	hook_result result(STATUS::OK);
	result.entry = /*boost::addressof(entry->second)*/entry;
	return result;
}
#if 0
auto minhook::create_hook_win_api(LPCWSTR pszModule, LPCSTR pszProcName, LPVOID pDetour) -> hook_result
{
    const auto hModule = GetModuleHandleW(pszModule);
    if (hModule == nullptr)
        return STATUS::ERROR_MODULE_NOT_FOUND;

    const auto target = static_cast<LPVOID>(GetProcAddress(hModule, pszProcName));
    if (target == nullptr)
        return STATUS::ERROR_FUNCTION_NOT_FOUND;

    return create_hook(target, pDetour);
}
#endif
void context::remove_all_hooks( )
{
	this->Reset_all_( );
}

STATUS context::enable_all_hooks( )
{
	return this->Set_state_all_(true);
}

STATUS context::disable_all_hooks( )
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
				return addressof(h);
		}
	}

	return nullptr;
}

STATUS context::Set_state_all_(bool enable, bool ignore_errors)
{
	//auto frozen = frozen_threads_storage(false);

	auto storage_active = storage__ | ranges::views::filter([](const hook_entry& h)
	{
		return h.target != nullptr;
	});
	const auto begin = storage_active.begin( );
	const auto end = storage_active.end( );

	for (auto itr_main = begin; itr_main != end; ++itr_main)
	{
		auto& value = *itr_main;
		if (value.enabled == enable)
			continue;

#if 0
		if (pause_threads)
		{
			//fill only if any hook enabled
			frozen.fill( );
		}
#endif

#ifdef _DEBUG
		auto main_status = STATUS::UNKNOWN;
		try
		{
			main_status = value.set_state(enable);
			if (main_status != STATUS::OK)
				BOOST_ASSERT("Error in code");
			continue;
		}
		catch ([[maybe_unused]] const std::runtime_error& e)
		{
			if (ignore_errors)
				continue;
		}
#else
		const auto main_status = value.set_state(enable);
		if (main_status == STATUS::OK)
			continue;
		if (ignore_errors)
			continue;
#endif

		//restore changes back
		enable = !enable;
		for (auto itr_child = begin; itr_child != itr_main; ++itr_child)
		{
			auto& value_child = *itr_child;
			if (value_child.enabled == enable)
				continue;
			if (const auto temp_status = value_child.set_state(enable); temp_status != STATUS::OK)
			{
				BOOST_ASSERT("Unable to revert hook state!");
				return temp_status;
			}
		}
		return main_status;
	}

	return STATUS::OK;
}

STATUS context::Reset_all_( )
{
	const auto result = Set_state_all_(false, true);
	storage__.clear( );
	return result;
}

auto _Generate_status_to_string_data( )
{
	using status_enum = STATUS;
	using raw_status_type = std::underlying_type_t<status_enum>;
#define STATUS_STR(x) data[(raw_status_type)status_enum::x].first = /*CRYPT_STR*/(#x);

	using namespace cheat;

	auto data = array<pair<string_view, status_enum>, static_cast<raw_status_type>(status_enum::COUNT)>( );
	for (raw_status_type i = 0; i < data.size( ); ++i)
	{
		data[i].second = static_cast<status_enum>(i);
	}

	STATUS_STR(UNKNOWN)
	STATUS_STR(OK)
	//STATUS_STR(ERROR_ALREADY_INITIALIZED)
	//STATUS_STR(ERROR_NOT_INITIALIZED)
	STATUS_STR(ERROR_ALREADY_CREATED)
	STATUS_STR(ERROR_NOT_CREATED)
	STATUS_STR(ERROR_ENABLED)
	STATUS_STR(ERROR_DISABLED)
	STATUS_STR(ERROR_NOT_EXECUTABLE)
	STATUS_STR(ERROR_UNSUPPORTED_FUNCTION)
	STATUS_STR(ERROR_MEMORY_ALLOC)
	STATUS_STR(ERROR_MEMORY_PROTECT)
	STATUS_STR(ERROR_MODULE_NOT_FOUND)
	STATUS_STR(ERROR_FUNCTION_NOT_FOUND)
	STATUS_STR(ERROR_VTABLE_NOT_FOUND)
	STATUS_STR(ERROR_NOT_READABLE)

#undef STATUS_STR

#ifdef _DEBUG
	for (const auto& key: data | ranges::views::keys)
	{
		if (key == std::remove_cvref_t<decltype(key)>( ))
			BOOST_ASSERT("Element isn't set!");
	}
#endif

	return data;
}

string_view hooks::status_to_string(STATUS status)
{
	static const/*expr*/ auto cache = _Generate_status_to_string_data( );

	try
	{
		return cache[static_cast<std::underlying_type_t<STATUS>>(status)].first;
	}
	catch (...)
	{
		return /*CRYPT_STR*/"Unsupported status";
	}
}

#ifdef _DEBUG
[[maybe_unused]] const auto _ = status_to_string(STATUS::OK);
#endif

hook_result context_safe::create_hook(LPVOID target, LPVOID detour)
{
	auto lock = make_lock_guard(mtx__);
	return ctx__.create_hook(target, detour);
}

STATUS context_safe::remove_hook(LPVOID target, bool force)
{
	auto lock = make_lock_guard(mtx__);
	return ctx__.remove_hook(target, force);
}

STATUS context_safe::enable_hook(LPVOID target)
{
	auto lock = make_lock_guard(mtx__);
	return ctx__.enable_hook(target);
}

STATUS context_safe::disable_hook(LPVOID target)
{
	auto lock = make_lock_guard(mtx__);
	return ctx__.disable_hook(target);
}

hook_result context_safe::find_hook(LPVOID target)
{
	auto lock = make_lock_guard(mtx__);
	return ctx__.find_hook(target);
}

void context_safe::remove_all_hooks( )
{
	auto lock = make_lock_guard(mtx__);
	ctx__.remove_all_hooks( );
}

STATUS context_safe::enable_all_hooks( )
{
	auto lock = make_lock_guard(mtx__);
	return ctx__.enable_all_hooks( );
}

STATUS context_safe::disable_all_hooks( )
{
	auto lock = make_lock_guard(mtx__);
	return ctx__.disable_all_hooks( );
}

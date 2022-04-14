module;

#include <nstd/runtime_assert.h>
#include <nstd/format.h>
#include <nstd/ranges.h>
#include <nstd/winapi/convert_string.h>

#include <windows.h>
#include <winternl.h>
#include <tchar.h>

#include <memory>
#include <string>
#include <functional>

module cheat.csgo.modules;
import cheat.console;
import nstd.mem.signature;
import nstd.mem.block;
import nstd.winapi.module_info;

using namespace nstd::mem;
namespace wp = nstd::winapi;

struct to_wide_string
{
	template<typename T>
	auto operator()(const std::basic_string_view<T> str) const noexcept
	{
		return _To_wide(str);
	}
};

struct extract_module_name
{
	auto operator()(LDR_DATA_TABLE_ENTRY* const ldr_entry)const noexcept
	{
		return wp::module_info(ldr_entry).name( );
	}

	template<typename T>
	auto operator()(const std::basic_string_view<T> str) const noexcept
	{
		return _To_wide(str);
	}
};

struct inform_found_pointer
{
	template<typename P>
	auto operator()(P* const object_ptr) const noexcept
	{
		return object_ptr ? std::format(_T("found at {:#X}"), reinterpret_cast<uintptr_t>(object_ptr)) : _T("not found");
	}
};

template<typename Mod, typename Obj, typename P>
static void _Console_log(const Mod module_name, const std::string_view object_type, const std::basic_string_view<Obj> object_name, P* const object_ptr) noexcept
{
	cheat::console::log(
		_T("{} -> {} \"{}\" {}"),
		std::bind_front(extract_module_name( ), module_name),
		std::bind_front(to_wide_string( ), object_type),
		std::bind_front(to_wide_string( ), object_name),
		std::bind_front(inform_found_pointer( ), object_ptr)
	);
}

void console_log(const _Strv module_name, const std::string_view object_type, const _Strv object_name, const basic_address<void> object_ptr) noexcept
{
	_Console_log(module_name, object_type, object_name, object_ptr.pointer);
}

#define console_log(...) static_assert(false,"console_log: use _Console_log instead")

void logs_writer::operator()(LDR_DATA_TABLE_ENTRY* const ldr_entry, const _Strv module_name) const noexcept
{
	cheat::console::log(_T("module \"{}\" found at {:#X}"), module_name, reinterpret_cast<uintptr_t>(ldr_entry));
}

void logs_writer::operator()(IMAGE_SECTION_HEADER* const sec, const _Strv module_name, const std::string_view section_name) const noexcept
{
	_Console_log(module_name, "section", section_name, sec);
}

uint8_t* find_signature_impl(LDR_DATA_TABLE_ENTRY* const ldr_entry, const std::string_view sig) noexcept
{
	//base address
	const basic_address<IMAGE_DOS_HEADER> dos = ldr_entry->DllBase;
	const basic_address<IMAGE_NT_HEADERS> nt = dos + dos->e_lfanew;

	const block mem = {dos.get<uint8_t*>( ), nt->OptionalHeader.SizeOfImage};
	const auto bytes = make_signature(sig.data( ), sig.size( ));
	const auto ret = mem.find_block(bytes);

	const auto result = ret.data( );
	_Console_log(ldr_entry, "signature", sig, result);
	return result;
}

struct interface_reg
{
	using instance_fn = void* (*)();
	instance_fn create_fn;
	const char* name;
	interface_reg* next;
};

static interface_reg* _Find_interface(const std::string_view name, interface_reg* const first, interface_reg* const last = nullptr) noexcept
{
	for (auto reg = first; reg != last; reg = reg->next)
	{
		if (std::memcmp(reg->name, name.data( ), name.size( )) != 0)
			continue;
		const auto last_char = reg->name[name.size( )];
		if (last_char != '\0' && !std::isdigit(last_char))
			continue;
		return reg;
	}

	return nullptr;
}

void* find_interface_impl(LDR_DATA_TABLE_ENTRY* const ldr_entry, const basic_address<void> create_interface_fn, const std::string_view name) noexcept
{
	interface_reg* const root_reg = create_interface_fn./*rel32*/jmp(0x5).plus(0x6).deref<2>( );
	interface_reg* const target_reg = _Find_interface(name, root_reg);
	runtime_assert(target_reg != nullptr);
	runtime_assert(_Find_interface(name, target_reg->next) == nullptr);
	const auto ifc_addr = std::invoke(target_reg->create_fn);
	_Console_log(ldr_entry, "interface", name, ifc_addr);
	return ifc_addr;
}

//----

static auto _Current_module_name( ) noexcept
{
	const wp::module_info info = wp::current_module( );
	return info.name( );
}

_Strv current_module::_Name( ) const noexcept
{
	static auto name = _Current_module_name( );
	return name;
}

interface_finder<current_module> current_module::_Ifc_finder(const basic_address<void> addr) const noexcept
{
	return addr;
}
module;

#include <nstd/runtime_assert.h>
#include <nstd/format.h>
#include <nstd/ranges.h>

#include <windows.h>
#include <winternl.h>

#include <memory>
#include <string>
#include <functional>

module cheat.csgo.modules;
import cheat.console;
import nstd.mem.signature;
import nstd.mem.block;

using namespace nstd::mem;

template<typename Mod, typename T>
static void _Console_log(const Mod module_name, const std::string_view object_type, const std::string_view object_name, T* object_ptr)
{
	cheat::console::log("{} -> {} \"{}\" {}", module_name, object_type, object_name, [=]
	{
		return object_ptr ? std::format("found at {:#X}", reinterpret_cast<uintptr_t>(object_ptr)) : "not found";
	});
}

void console_log(const std::string_view module_name, const std::string_view object_type, const std::string_view object_name, const basic_address<void> object_ptr)
{
	_Console_log(module_name, object_type, object_name, object_ptr.pointer);
}

void console_log(const std::function<std::string_view( )> module_name_getter, const std::string_view object_type, const std::string_view object_name, const basic_address<void> object_ptr)
{
	_Console_log(std::ref(module_name_getter), object_type, object_name, object_ptr.pointer);
}

#define console_log(...) static_assert(false,"console_log: use _Console_log instead")

void logs_writer::operator()(LDR_DATA_TABLE_ENTRY* entry, const std::string_view module_name) const
{
	cheat::console::log("module \"{}\" found at {:#X}", module_name, reinterpret_cast<uintptr_t>(entry));
}

void logs_writer::operator()(IMAGE_SECTION_HEADER* const sec, const std::string_view module_name, const std::string_view section_name) const
{
	_Console_log(module_name, "section", section_name, sec);
}

static std::string _Get_module_name(LDR_DATA_TABLE_ENTRY* ldr_entry)
{
	const std::basic_string_view full_path = {ldr_entry->FullDllName.Buffer, ldr_entry->FullDllName.Length / sizeof(WCHAR)};
	const auto name_start = full_path.find_last_of('\\');
	const auto wname = full_path.substr(name_start + 1);
#pragma warning(push)
#pragma warning(disable:4244)
	std::string out;
	out.reserve(wname.size( ));
	//it internally call push_back because of incompatible (WCHAR) type
	out.assign(wname.begin( ), wname.end( ));
	return out;
#pragma warning(pop)
}

uint8_t* find_signature_impl(LDR_DATA_TABLE_ENTRY* ldr_entry, const std::string_view sig)
{
	//base address
	const basic_address<IMAGE_DOS_HEADER> dos = ldr_entry->DllBase;
	const basic_address<IMAGE_NT_HEADERS> nt = dos + dos->e_lfanew;

	const block mem = {dos.get<uint8_t*>( ), nt->OptionalHeader.SizeOfImage};
	const auto bytes = make_signature(sig.data( ), sig.size( ));
	const auto ret = mem.find_block(bytes);

	const auto result = ret.data( );
	_Console_log(std::bind_front(_Get_module_name, ldr_entry), "signature", sig, result);
	return result;
}

struct interface_reg
{
	using instance_fn = void* (*)();
	instance_fn create_fn;
	const char* name;
	interface_reg* next;
};

static interface_reg* _Find_interface(const std::string_view name, interface_reg* const first, const interface_reg* last = nullptr)
{
	for (auto reg = first; reg != last; reg = reg->next)
	{
		if (std::memcmp(reg->name, name.data( ), name.size( )) != 0)
			continue;
		if (!std::isdigit(reg->name[name.size( )]))
			continue;
		return reg;
	}

	return nullptr;
}

void* find_interface_impl(LDR_DATA_TABLE_ENTRY* ldr_entry, const basic_address<void> create_interface_fn, const std::string_view name)
{
	interface_reg* const root_reg = create_interface_fn./*rel32*/jmp(0x5).plus(0x6).deref<2>( );
	interface_reg* const target_reg = _Find_interface(name, root_reg);
	runtime_assert(target_reg != nullptr);
	runtime_assert(_Find_interface(name, target_reg->next) == nullptr);
	const auto ifc_addr = std::invoke(target_reg->create_fn);
	_Console_log(std::bind_front(_Get_module_name, ldr_entry), "interface", name, ifc_addr);
	return ifc_addr;
}
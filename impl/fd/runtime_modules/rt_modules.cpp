module;

#include <fd/core/assert.h>

#include <windows.h>
#include <winternl.h>

#include <functional>
#include <memory>
#include <string>

module fd.rt_modules;
/* import fd.logger.system_console;
import nstd.mem.signature;
import nstd.mem.block;
import nstd.winapi.module_info;
import nstd.winapi.helpers;

using namespace nstd::mem;
namespace wp = nstd::winapi;

struct extract_module_name
{
    auto operator()(LDR_DATA_TABLE_ENTRY* const ldr_entry) const
    {
        return wp::module_info(ldr_entry).name();
    }

    template <typename... Ts>
    auto operator()(const std::basic_string_view<Ts...> str) const
    {
        return str;
    }
};

struct inform_found_pointer
{
    auto operator()(const basic_address<void> object_ptr) const
    {
        return object_ptr ? nstd::format("found at {:#X}", object_ptr.value) : "not found";
    }
};

template <class C, typename... Args>
static auto _Bind_front(Args&&... args)
{
    return std::bind_front(C(), std::forward<Args>(args)...);
}

template <typename Mod, typename ObjT, typename ObjN, typename P>
static void _Console_log(const Mod module_name, const ObjT& object_type, const ObjN& object_name, P* const object_ptr)
{
    fd::logger_system_console->log(L"{} -> {} \"{}\" {}", _Bind_front<extract_module_name>(module_name), object_type, object_name, _Bind_front<inform_found_pointer>(object_ptr));
}

void console_log(const std::wstring_view module_name, const std::string_view object_type, const std::string_view object_name, const basic_address<void> object_ptr)
{
    _Console_log(module_name, object_type, object_name, object_ptr.pointer);
}

#define console_log(...) static_assert(false, "console_log: use _Console_log instead")

void logs_writer::operator()(LDR_DATA_TABLE_ENTRY* const ldr_entry, const std::wstring_view module_name) const
{
    fd::logger_system_console->log(L"module \"{}\" found at {:#X}", module_name, reinterpret_cast<uintptr_t>(ldr_entry));
}

void logs_writer::operator()(IMAGE_SECTION_HEADER* const sec, const std::wstring_view module_name, const std::string_view section_name) const
{
    _Console_log(module_name, "section", section_name, sec);
}

uint8_t* find_signature_impl(LDR_DATA_TABLE_ENTRY* const ldr_entry, const std::string_view sig)
{
    const auto [dos, nt] = wp::dos_nt(ldr_entry);

    const block mem  = {dos.get<uint8_t*>(), nt->OptionalHeader.SizeOfImage};
    const auto bytes = unknown_signature(sig.data(), sig.size());
    const auto ret   = mem.find_block(bytes);

    const auto result = ret.data();
    _Console_log(ldr_entry, "signature", sig, result);
    return result;
} */

//----

/* std::wstring_view current_module::_Name() const
{
    static const auto name = wp::module_info(wp::current_module()).name();
    return name;
}

interface_finder<current_module> current_module::_Ifc_finder(const basic_address<void> addr) const
{
    return addr;
}
 */

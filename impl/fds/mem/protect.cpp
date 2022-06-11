module;

#if 0
#include <nstd/FDS_ASSERT.h>
#endif
#include <Windows.h>

#include <memory>
#include <optional>
#include <string_view>

module fds.mem_protect;

#if 0
import nstd.mem.block;
#endif

struct local_free
{
    void operator()(LPSTR ptr) const
    {
        LocalFree(ptr);
    }
};

using local_buffer_base = std::unique_ptr<char, local_free>;

struct local_buffer : local_buffer_base
{
    local_buffer() = default;

    local_buffer(LPSTR msg)
        : local_buffer_base(msg)
    {
    }
};

class last_error_string
{
    DWORD        id;
    local_buffer buffer;
    size_t       msg_size;

  public:
    last_error_string()
    {
        // Get the error message ID, if any.
        id = ::GetLastError();
        if (id == 0)
            return; // No error message has been recorded

        LPSTR msg;
        // Ask Win32 to give us the string version of that message ID.
        // The parameters we pass in, tell Win32 to create the buffer that holds the message for us (because we don't yet know how long the message string will be).
        msg_size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, id, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)std::addressof(msg), 0, NULL);
        buffer   = msg;
    }

    std::string_view view() const
    {
        return {buffer.get(), msg_size};
    }
};

static DWORD _Set_flags(const LPVOID addr, const SIZE_T size, const DWORD new_flags)
{
#if 0
	const nstd::mem::block block = {static_cast<uint8_t*>(addr),size};
	const auto old_flags = block.get_flags( );
	FDS_ASSERT(old_flags != 0);
	DWORD old_flags2 = 0;
	//does not work
	if (VirtualProtect(addr, size, old_flags | new_flags, std::addressof(old_flags2)))
	{
		FDS_ASSERT(old_flags == old_flags2);
		return old_flags;
	}
#else
    DWORD old_flags;
    if (VirtualProtect(addr, size, new_flags, std::addressof(old_flags)))
        return old_flags;
#endif

#ifdef _DEBUG
    [[maybe_unused]] const last_error_string error;
#endif

    return 0;
}

DWORD mem_protect::data::set() const
{
    return _Set_flags(addr, size, flags);
}

mem_protect::mem_protect() = default;

mem_protect::mem_protect(const LPVOID addr, const SIZE_T size, const DWORD new_flags)
{
    const auto old_flags = _Set_flags(addr, size, new_flags);
    if (old_flags != 0)
        info_.emplace(addr, size, old_flags);
}

mem_protect::mem_protect(mem_protect&& other)
{
    *this = std::move(other);
}

mem_protect::~mem_protect()
{
    if (!info_.has_value())
        return;
    info_->set();
}

mem_protect& mem_protect::operator=(mem_protect&& other)
{
    using std::swap;
    swap(this->info_, other.info_);
    return *this;
}

bool mem_protect::restore()
{
    if (!info_.has_value())
        return false;
    const auto old_flags = info_->set();
    if (old_flags == 0)
        return false;
    info_.reset();
    return true;
}

bool mem_protect::has_value() const
{
    return info_.has_value();
}

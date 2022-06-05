module;

#include <windows.h>

#include <optional>

export module fds.mem_protect;

class mem_protect
{
  public:
    struct data
    {
        LPVOID addr;
        SIZE_T size;
        DWORD  flags;

        DWORD set() const;
    };

    using value_type = std::optional<data>;

    mem_protect();
    mem_protect(const LPVOID addr, const SIZE_T size, const DWORD new_flags);
    mem_protect(mem_protect&& other);
    ~mem_protect();

    mem_protect(const mem_protect&)            = delete;
    mem_protect& operator=(const mem_protect&) = delete;
    mem_protect& operator=(mem_protect&& other);

    bool restore();
    bool has_value() const;

  private:
    value_type info_;
};

export namespace fds
{
    using ::mem_protect;
}

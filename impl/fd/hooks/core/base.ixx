module;

#include <string_view>

export module fd.hook_base;

struct hook_base
{
    virtual ~hook_base() = default;

    virtual bool enable()  = 0;
    virtual bool disable() = 0;

    virtual bool is_static() const        = 0;
    virtual std::string_view name() const = 0;

    virtual bool initialized() const = 0;
    virtual bool active() const      = 0;

    virtual void* get_original_method() const = 0;
    virtual void init()                       = 0;
};

export namespace fd
{
    using ::hook_base;
} // namespace fd

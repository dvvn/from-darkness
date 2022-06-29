module;

#include <fd/object.h>

#include <algorithm>
#include <array>

export module fd.hooks_loader;
import fd.hook_base;

constexpr bool know_hook(const size_t i)
{
#ifdef FD_KNOWN_HOOKS
    const std::array known = { FD_KNOWN_HOOKS };
    return std::find(known.begin(), known.end(), i) != known.end();
#else
    return true;
#endif
}

template <size_t... I>
constexpr bool is_unique(const std::index_sequence<I...>)
{
    if constexpr (sizeof...(I) == 1)
        return true;
    else
    {
        std::array tmp = { I... };
        std::sort(tmp.begin(), tmp.end());
        return std::adjacent_find(tmp.begin(), tmp.end()) == tmp.end();
    }
}

struct basic_hooks_loader
{
    using hook_base = fd::hook_base;

    virtual ~basic_hooks_loader() = default;

    template <size_t... I>
    bool load(const bool stop_on_error = true, const std::index_sequence<I...> seq = {})
    {
        static_assert(is_unique(seq));
        static_assert((know_hook(I) && ...));
        (store(&FD_OBJECT_GET(hook_base, I)), ...);
        return init(stop_on_error);
    }
#ifdef FD_KNOWN_HOOKS
    bool load(const bool stop_on_error = true)
    {
        return load<FD_KNOWN_HOOKS>(stop_on_error);
    }
#endif

    virtual void disable() = 0;
    virtual void unload() = 0;

  protected:
    virtual bool init(const bool stop_on_error) = 0;
    virtual void store(hook_base* const hook) = 0;
};

FD_OBJECT(hooks_loader, basic_hooks_loader);

export namespace fd
{
    using ::hooks_loader;
}

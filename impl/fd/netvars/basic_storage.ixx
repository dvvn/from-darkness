module;

#include <fd/utility.h>

#include <memory>
#include <vector>

export module fd.netvars.core:basic_storage;
export import fd.valve.recv_table;
export import fd.valve.data_map;
import fd.type_name;

class netvar_info_source
{
    using _Recv = fd::valve::recv_prop* const;
    using _Dmap = fd::valve::data_map_description* const;

    union
    {
        _Recv recv_;
        _Dmap dmap_;
    };

    uint8_t mode_;

  public:
    netvar_info_source(const _Recv recv)
        : recv_(recv)
        , mode_(1)
    {
    }

    netvar_info_source(const _Dmap dmap)
        : dmap_(dmap)
        , mode_(2)
    {
    }

    template <typename Fn>
    decltype(auto) operator()(Fn fn) const
    {
        switch (mode_)
        {
        case 1:
            if constexpr (std::invocable<Fn, _Recv>)
                return fd::invoke(fn, recv_);
        case 2:
            if constexpr (std::invocable<Fn, _Dmap>)
                return fd::invoke(fn, dmap_);
        default:
            fd::unreachable();
        }
    }
};

class basic_netvar_info
{
  public:
    virtual ~basic_netvar_info() = default;

    virtual size_t offset() const               = 0;
    virtual fd::hashed_string_view name() const = 0;
    virtual fd::string_view type() const        = 0;
};

class netvar_info final : public basic_netvar_info
{
    size_t offset_;
    netvar_info_source source_;
    size_t size_; // for arrays
    mutable fd::hashed_string_view name_;
    mutable fd::string type_;

  public:
    netvar_info(const size_t offset, const netvar_info_source source, const size_t size = 0, const fd::hashed_string_view name = {});

    size_t offset() const;
    fd::hashed_string_view name() const;
    fd::string_view type() const;
};

template <typename Fn>
union offset_getter
{
    Fn fn_;
    size_t offset_;

  public:
    offset_getter(Fn&& fn)
        : fn_(std::move(fn))
    {
    }

    void destroy()
    {
        if constexpr (std::is_class_v<Fn>)
            std::destroy_at(&fn_);
    }

    void set()
    {
        const auto value = fd::invoke(fn_);
        destroy();
        offset_ = value;
    }

    size_t get() const
    {
        return offset_;
    }
};

template <typename Fn>
class netvar_info_custom final : public basic_netvar_info
{
    mutable offset_getter<Fn> getter_;
    mutable bool have_offset_ = false;
    fd::hashed_string_view name_;
    fd::string type_;

  public:
    ~netvar_info_custom()
    {
        if (!have_offset_)
            getter_.destroy();
    }

    netvar_info_custom(Fn&& getter, const fd::hashed_string_view name = {}, fd::string&& type = {})
        : getter_(std::move(getter))
        , name_(name)
        , type_(std::move(type))
    {
    }

    size_t offset() const
    {
        if (!have_offset_)
        {
            getter_.set();
            have_offset_ = true;
        }
        return getter_.get();
    }

    fd::hashed_string_view name() const
    {
        return name_;
    }

    fd::string_view type() const
    {
        return type_;
    }
};

class netvar_info_custom_constant final : public basic_netvar_info
{
    size_t offset_;
    fd::hashed_string_view name_;
    fd::string type_;

  public:
    netvar_info_custom_constant(const size_t offset, const fd::hashed_string_view name = {}, fd::string&& type = {});

    size_t offset() const;
    fd::hashed_string_view name() const;
    fd::string_view type() const;
};

class netvar_table : public std::vector<std::unique_ptr<basic_netvar_info>>
{
    void validate_item(const basic_netvar_info* info) const;

    template <typename T, typename... Args>
    const T* add_impl(Args&&... args)
    {
        auto uptr    = std::make_unique<T>(std::forward<Args>(args)...);
        const T* ret = uptr.get();
        this->emplace_back(std::move(uptr));
        this->validate_item(ret);
        return ret;
    }

    fd::hashed_string name_;

  public:
    netvar_table(fd::hashed_string&& name);

    fd::hashed_string_view name() const;

    const basic_netvar_info* find(const fd::hashed_string_view name) const;

    const netvar_info* add(const size_t offset, const netvar_info_source source, const size_t size = 0, const fd::hashed_string_view name = {});
    const netvar_info_custom_constant* add(const size_t offset, const fd::hashed_string_view name, fd::string&& type = {});

    template <std::invocable Fn>
    auto add(Fn&& offset_getter, const fd::hashed_string_view name, fd::string&& type = {})
    {
        return add_impl<netvar_info_custom<std::remove_cvref_t<Fn>>>(std::forward<Fn>(offset_getter), name, std::move(type));
    }

    template <typename Type, typename TypeProj = std::identity, typename From>
    auto add(From&& from, const fd::hashed_string_view name, TypeProj proj = {})
    {
        return add(std::forward<From>(from), name, fd::invoke(proj, fd::type_name<Type>()));
    }
};

export namespace fd::netvars
{
    struct basic_storage : std::vector<netvar_table>
    {
        const netvar_table* find(const fd::hashed_string_view name) const;
        netvar_table* find(const fd::hashed_string_view name);

        netvar_table* add(netvar_table&& table, const bool skip_find = false);
        netvar_table* add(fd::hashed_string&& name, const bool skip_find = false);
    };
} // namespace fd::netvars

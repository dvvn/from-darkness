module;

#include <concepts>
#include <variant>

export module fd.netvars.info;
export import fd.valve.recv_table;
export import fd.valve.data_map;
export import fd.string;
export import fd.functional.invoke;

using fd::string;
using fd::string_view;

using hashed_string_view = string_view;
using hashed_string      = string;

class basic_netvar_info
{
  public:
    virtual ~basic_netvar_info() = default;

    virtual size_t offset() const           = 0;
    virtual hashed_string_view name() const = 0;
    virtual string_view type() const        = 0;
};

using fd::valve::data_map_description;
using fd::valve::recv_prop;

using netvar_info_source = std::variant<const data_map_description*, const recv_prop*>;

class netvar_info final : public basic_netvar_info
{
    size_t offset_;
    netvar_info_source source_;
    size_t size_; // for arrays
    mutable hashed_string_view name_;
    mutable string type_;

  public:
    netvar_info(const size_t offset, const netvar_info_source source, const size_t size = 0, const hashed_string_view name = {});

    size_t offset() const;
    hashed_string_view name() const;
    string_view type() const;
};

using fd::invoke;

template <typename Fn>
class netvar_info_lazy final : public basic_netvar_info
{
    mutable std::variant<size_t, Fn> getter_;
    hashed_string_view name_;
    string type_;

  public:
    netvar_info_lazy(Fn&& getter, const hashed_string_view name = {}, string&& type = {})
        : getter_(std::move(getter))
        , name_(name)
        , type_(std::move(type))
    {
    }

    size_t offset() const
    {
        if (std::holds_alternative<Fn>(getter_))
        {
            const auto offset = invoke(std::get<Fn>(getter_));
            return getter_.emplace(offset);
        }

        return std::get<size_t>(getter_);
    }

    hashed_string_view name() const
    {
        return name_;
    }

    string_view type() const
    {
        return type_;
    }
};

template <typename Fn>
netvar_info_lazy(const Fn&) -> netvar_info_lazy<Fn>;

class netvar_info_instant final : public basic_netvar_info
{
    size_t offset_;
    hashed_string_view name_;
    string type_;

  public:
    netvar_info_instant(const size_t offset, const hashed_string_view name = {}, string&& type = {});

    size_t offset() const;
    hashed_string_view name() const;
    string_view type() const;
};

template <typename T, typename...>
using first_arg = T;

template <typename... Args>
auto make_netvar_info(Args&&... args)
{
    using arg1 = first_arg<Args...>;
    if constexpr (invocable<arg1>)
        return new netvar_info_lazy(std::forward<Args>(args)...);
    else if constexpr (std::constructible_from<netvar_info, Args&&...>)
        return new netvar_info(std::forward<Args>(args)...);
    else if constexpr (std::constructible_from<netvar_info_instant, Args&&...>)
        return new netvar_info_instant(std::forward<Args>(args)...);
}

export namespace fd
{
    using ::basic_netvar_info;
    using ::netvar_info;
    using ::netvar_info_instant;
    using ::netvar_info_lazy;

    using ::make_netvar_info;
} // namespace fd

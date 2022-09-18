module;

#include <concepts>
#include <variant>

export module fd.netvars.info;
export import fd.valve.recv_table;
export import fd.valve.data_map;
export import fd.string;
export import fd.functional.invoke;

namespace fd
{
    export struct basic_netvar_info
    {
        virtual ~basic_netvar_info() = default;

        virtual size_t offset() const    = 0;
        virtual string_view name() const = 0;
        virtual string_view type() const = 0;
    };

    using netvar_info_source = std::variant<const valve::data_map_description*, const valve::recv_prop*>;

    export class netvar_info final : public basic_netvar_info
    {
        size_t offset_;
        netvar_info_source source_;
        size_t size_; // for arrays
        mutable string_view name_;
        mutable string type_;

      public:
        netvar_info(const size_t offset, const netvar_info_source source, const size_t size = 0, const string_view name = {});

        size_t offset() const;
        string_view name() const;
        string_view type() const;
    };

    export template <typename Fn>
    class netvar_info_lazy final : public basic_netvar_info
    {
        mutable std::variant<size_t, Fn> getter_;
        string_view name_;
        string type_;

      public:
        netvar_info_lazy(Fn&& getter, const string_view name = {}, string&& type = {})
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

        string_view name() const
        {
            return name_;
        }

        string_view type() const
        {
            return type_;
        }
    };

    template <typename Fn>
    netvar_info_lazy(Fn&&) -> netvar_info_lazy<std::decay_t<Fn>>;

    export class netvar_info_instant final : public basic_netvar_info
    {
        size_t offset_;
        string_view name_;
        string type_;

      public:
        netvar_info_instant(const size_t offset, const string_view name = {}, string&& type = {});

        size_t offset() const;
        string_view name() const;
        string_view type() const;
    };

#define _CHECK(_CLASS_) std::constructible_from<_CLASS_, Arg1&&, Args&&...>
#define _RET(_CLASS_)   new _CLASS_(std::forward<Arg1>(arg1), std::forward<Args>(args)...)

  export template <typename Arg1, typename... Args>
  auto make_netvar_info(Arg1&& arg1, Args&&... args)
  {
        if constexpr (invocable<Arg1>)
            return _RET(netvar_info_lazy);
        else if constexpr (_CHECK(netvar_info))
            return _RET(netvar_info);
        else if constexpr (_CHECK(netvar_info_instant))
            return _RET(netvar_info_instant);
  }
} // namespace fd

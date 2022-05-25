module;

#include <vector>
#include <string>
#include <variant>
#include <memory>

export module cheat.netvars.core:basic_storage;
export import cheat.csgo.structs.Recv;
export import cheat.csgo.structs.DataMap;
import cheat.tools.object_name;
import nstd.text.hashed_string;
import nstd.text.string_or_view;

using netvar_info_source = std::variant<cheat::csgo::RecvProp*, cheat::csgo::typedescription_t*>;
using nstd::text::string_or_view;
using nstd::text::hashed_string_view;
using nstd::text::hashed_string;

class basic_netvar_info
{
public:
	virtual ~basic_netvar_info( ) = default;

    virtual size_t offset() const = 0;
    virtual hashed_string_view name() const = 0;
    virtual std::string_view type() const = 0;
};

class netvar_info final :public basic_netvar_info
{
	size_t offset_;
	netvar_info_source source_;
	size_t size_;//for arrays
	mutable hashed_string_view name_;
	mutable string_or_view type_;

public:
	netvar_info(const size_t offset, const netvar_info_source source, const size_t size = 0, const hashed_string_view name = {});

    size_t offset() const;
    hashed_string_view name() const;
    std::string_view type() const;
};

template<typename Fn>
class netvar_info_custom final :public basic_netvar_info
{
	mutable std::variant<size_t, Fn> getter_;
	hashed_string_view name_;
	string_or_view type_;

public:
	netvar_info_custom(Fn&& getter, const hashed_string_view name = {}, string_or_view&& type = {})
		:getter_(std::move(getter)), name_(name), type_(type)
	{
    }

    size_t offset() const
    {
		if(std::holds_alternative<size_t>(getter_))
			return std::get<0>(getter_);

		const auto offset = std::invoke(std::get<1>(getter_));
		getter_ = offset;
		return offset;
    }

    hashed_string_view name() const
    {
		return name_;
    }

    std::string_view type() const
    {
		return type_;
	}
};

class netvar_info_custom_constant final :public basic_netvar_info
{
	size_t offset_;
	hashed_string_view name_;
	string_or_view type_;

public:
	netvar_info_custom_constant(const size_t offset, const hashed_string_view name = {}, string_or_view&& type = {});

    size_t offset() const;
    hashed_string_view name() const;
    std::string_view type() const;
};

class netvar_table : public std::vector<std::unique_ptr<basic_netvar_info>>
{
    void validate_item(const basic_netvar_info* info) const;

    template <typename T, typename... Args>
    const T* add_impl(Args&&... args)
    {
		auto uptr = std::make_unique<T>(std::forward<Args>(args)...);
		const T* ret = uptr.get( );
		this->emplace_back(std::move(uptr));
		this->validate_item(ret);
		return ret;
	}

	hashed_string name_;

public:
	netvar_table(hashed_string&& name);

    hashed_string_view name() const;

    const basic_netvar_info* find(const hashed_string_view name) const;

    const netvar_info* add(const size_t offset, const netvar_info_source source, const size_t size = 0, const hashed_string_view name = {});
    const netvar_info_custom_constant* add(const size_t offset, const hashed_string_view name, string_or_view&& type = {});

    template <std::invocable Fn>
    auto add(Fn&& getter, const hashed_string_view name, string_or_view&& type = {})
    {
		return add_impl<netvar_info_custom<std::remove_cvref_t<Fn>>>(std::forward<Fn>(getter), name, std::move(type));
    }

    template <typename Type, typename TypeProj = std::identity, typename From>
    auto add(From&& from, const hashed_string_view name, TypeProj proj = {})
    {
		return add(std::forward<From>(from), name, std::invoke(proj, cheat::tools::csgo_object_name<Type>));
	}
};

export namespace cheat::netvars
{
	struct basic_storage : std::vector<netvar_table>
	{
        const netvar_table* find(const hashed_string_view name) const;
        netvar_table* find(const hashed_string_view name);

        template <typename T>
        auto find()
        {
			return this->find(tools::csgo_object_name<T>);
        }

        netvar_table* add(netvar_table&& table, const bool skip_find = false);
        netvar_table* add(hashed_string&& name, const bool skip_find = false);
    };
}
